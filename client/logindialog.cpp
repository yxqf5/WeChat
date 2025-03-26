#include "logindialog.h"
#include "ui_logindialog.h"
#include"httpmgr.h"
#include"tcpmgr.h"
#include "usermgr.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->reg_btn,&QPushButton::clicked,this,&LoginDialog::switchRegister);

    initHttpHandlers();
    ui->resetpass_lab->SetState("normal","hover","","selected","selected_hover","");
    ui->resetpass_lab->setCursor(Qt::PointingHandCursor);

    connect(ui->resetpass_lab, &ClickedLabel::clicked, this, &LoginDialog::slot_resetpass);

    //登录界面的密码显示按钮
    ui->pass_lineEdit->setEchoMode(QLineEdit::Password);
    ui->pass_visible_label->SetState("unvisible","unvisible_hover","","visible",
                                    "visible_hover","");
    //连接点击事件
    connect(ui->pass_visible_label, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible_label->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pass_lineEdit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_lineEdit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });


    //HttpMgr发出的sig_login_mod_finish信号接收,连接
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish,
            this, &LoginDialog::slot_login_mod_finish);




    //tcpMgr长连接的信号相关
    //发出tcp连接请求
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);

    //响应tcpMgr发出的连接成功信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_con_finish);

}

LoginDialog::~LoginDialog()
{
    qDebug()<<"destruct LoginDlg";
    delete ui;
}

void LoginDialog::slot_resetpass()
{
    qDebug()<<"slot forget pwd";
    emit switchReset();
}

void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"login button clicked";
    if(false == checkUserValid()){
        return;
    }

    if(false == checkPasswordValid()){
        return;
    }

    QString user = ui->user_lineEdit->text();
    QString passwd = ui->pass_lineEdit->text();

    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = xorString(passwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);


}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    if(err == ErrorCodes::ERR_NETWORK){
        showTip(tr("网路请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }
    //根据id调用回调函数
    _handlers[id](jsonDoc.object());
    return;
}



void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }

        auto user = jsonObj["user"].toString();

        //发送信号通知tcpMgr发送长链接
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();

        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "user is " << user << " uid is " << si.Uid <<" host is "
                 << si.Host << " Port is " << si.Port << " Token is " << si.Token;

        UserMgr::GetInstance()->SetUid(si.Uid);
        // UserMgr::GetInstance()->SetUid(si.Uid);
        // UserMgr::GetInstance()->SetUid(si.Uid);

        emit sig_connect_tcp(si);
    });
}

void LoginDialog::showTip(QString str,bool state)
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



bool LoginDialog::checkUserValid(){

    auto user = ui->user_lineEdit->text();
    if(user.isEmpty()){
        qDebug() << "User empty " ;
        return false;
    }

    return true;
}

bool LoginDialog::checkPasswordValid(){
    auto pwd = ui->pass_lineEdit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pass length invalid";
        return false;
    }

    return true;
}


void LoginDialog::slot_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip("聊天服务连接成功，正在登录...",true);
        QJsonObject json_obj;
        json_obj["uid"] = _uid;
        json_obj["token"] = _token;

        QJsonDocument doc(json_obj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);
        qDebug()<<"the send data is "<<jsonString;
        //发出发送消息信号
       emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);

    }
    else
    {
        showTip("网络异常tcp",false);
        enableBtn(false);
    }

}


//登录失败
void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登录失败, err is % 1").arg(err);
}

bool LoginDialog::enableBtn(bool enabled){

    ui->login_btn->setEnabled(enabled);
    ui->reg_btn->setEnabled(enabled);
    return true;

}

