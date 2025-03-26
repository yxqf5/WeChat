#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include"httpmgr.h"


RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog),_countdown(5)
{
    ui->setupUi(this);
    ui->pass_lineEdit->setEchoMode(QLineEdit::Password);
    ui->passagain_lineEdit->setEchoMode(QLineEdit::Password);

    ui->error_tip->setProperty("state","normal");
    repolish(ui->error_tip);


    initHttpHandlers();

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish,
            this, &RegisterDialog::slot_reg_mod_finish);

    // connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);

    //清理开始的错误提示
    ui->error_tip->clear();


    connect(ui->user_lineEdit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->email_lineEdit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->pass_lineEdit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });

    connect(ui->passagain_lineEdit, &QLineEdit::editingFinished, this, [this](){
        checkConfirmValid();
    });

    connect(ui->code_lineEdit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });

    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->passagain_visible->setCursor(Qt::PointingHandCursor);


    ui->pass_visible->SetState("unvisible","unvisible_hover","","visible",
                               "visible_hover","");

    ui->passagain_visible->SetState("unvisible","unvisible_hover","","visible",
                                  "visible_hover","");


    //连接点击事件

    connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pass_lineEdit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_lineEdit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    connect(ui->passagain_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->passagain_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->passagain_lineEdit->setEchoMode(QLineEdit::Password);
        }else{
            ui->passagain_lineEdit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });


    //倒计时返回登录
    // 创建定时器
    _countdown_timer = new QTimer(this);
    // 连接信号和槽
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        ui->tip1_lab->setText(str);
    });



}

RegisterDialog::~RegisterDialog()
{
    qDebug()<<"destruct RegisterDialog";
    delete ui;
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_4);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}

void RegisterDialog::showTip(QString str,bool state)
{

    if(state){
        ui->error_tip->setProperty("state","normal");
    }
    else
    {
        ui->error_tip->setProperty("state","err");

    }

    ui->error_tip->setText(str);
        repolish(ui->error_tip);
}


//答案:你遇到的问题很可能是因为Qt的 自动信号槽连接机制 在起作用。Qt 有一个特性：当槽函数按照 on_控件对象名_信号名 的规则命名时，会自动连接对应的信号和槽，而无需手动调用 connect。
//有个疑问,这里的槽函数好像没有被connect绑定到获取按钮上,为什么会触发?
//获取验证码时,需要验证邮箱的正确性
void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_lineEdit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();

    if(match){
        //发送http请求,获取验证码,短连接;
        QJsonObject json_obj;
        json_obj["email"] = email;
        qDebug()<<gate_url_prefix;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_varrfycode"), json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);


    }else
    {
        showTip(tr("邮件格式不正确"),false);
    }

}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err == ErrorCodes::ERR_NETWORK){
        showTip(tr("网路请求错误"),false);
        return;
    }

    //解析Json字符串, res转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("json 解析失败"),false);
        return;
    }

    if(!jsonDoc.isObject()){
        showTip(tr("json 解析错误"),false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;

}

//注册确认按钮
void RegisterDialog::on_yes_pbt_clicked()
{
    bool valid = checkUserValid();
    if(!valid){
        return;
    }

    valid = checkEmailValid();
    if(!valid){
        return;
    }

    valid = checkPassValid();
    if(!valid){
        return;
    }

    valid = checkVarifyValid();
    if(!valid){
        return;
    }

    //发送http注册用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_lineEdit->text();
    json_obj["email"] = ui->email_lineEdit->text();
    json_obj["passwd"] = xorString(ui->pass_lineEdit->text());
    json_obj["varifycode"] = ui->code_lineEdit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}


void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj){ // 这里捕获的this是用来干什么的??为什么可以就这样捕获?
        int error  = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS)
        {
            showTip(tr("参数错误"),false);
            return;
        }

        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱,请注意查收!"),true);
        qDebug()<<"email is "<<email;

    });

    _handlers.insert(ReqId::ID_REG_USER, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }

        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"),true);
        qDebug()<<"Email is "<<email;
        ChangeTipPage();

    });

}



//检测注册的输入框是否合法
bool RegisterDialog::checkUserValid()
{
    if(ui->user_lineEdit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}




bool RegisterDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_lineEdit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->pass_lineEdit->text();

    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    if(ui->pass_lineEdit->text() != ui->passagain_lineEdit->text()){

        AddTipErr(TipErr::TIP_CONFIRM_ERR, tr("确认密码错误"));

        return false;
    }

    DelTipErr(TipErr::TIP_CONFIRM_ERR);
    return true;
}


bool RegisterDialog::checkVarifyValid()
{
    auto pass = ui->code_lineEdit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}


//向map中添加注册框内容的错误,并显示第一个错误就行;
void RegisterDialog::AddTipErr(TipErr te, QString tips)
{
    m_tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::DelTipErr(TipErr te)
{
    m_tip_errs.remove(te);
    if(m_tip_errs.empty()){
        ui->error_tip->clear();
        return;
    }

    showTip(m_tip_errs.first(), false);
}





void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void RegisterDialog::on_no_pbt_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

