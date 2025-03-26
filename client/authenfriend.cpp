#include "authenfriend.h"
#include "ui_authenfriend.h"


AuthenFriend::AuthenFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthenFriend)
{
    ui->setupUi(this);


    connect(ui->enter_pushButton, &QPushButton::clicked, this, &AuthenFriend::SlotApplySure);
    connect(ui->return_pushButton, &QPushButton::clicked, this, &AuthenFriend::SlotApplyCancel);

}

AuthenFriend::~AuthenFriend()
{
    qDebug()<< "AuthenFriend destruct";
    delete ui;
}


void AuthenFriend::SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;
    ui->nick_name_lineEdit->setPlaceholderText(apply_info->_name);
    ui->con_name_lineEdit->setPlaceholderText(apply_info->_name);
    auto text = std::to_string(apply_info->_uid);
    ui->email_lineEdit->setPlaceholderText(text.data());

}


void AuthenFriend::SlotApplySure()
{
    qDebug() << "Slot Apply Sure ";
    //添加发送逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::GetInstance()->GetUid();
    jsonObj["fromuid"] = uid;
    jsonObj["touid"] = _apply_info->_uid;
    QString back_name = "";
    if(ui->nick_name_lineEdit->text().isEmpty()){
        back_name = ui->nick_name_lineEdit->placeholderText();
    }else{
        back_name = ui->nick_name_lineEdit->text();
    }
    jsonObj["back"] = back_name;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ, jsonData);

    this->hide();
    deleteLater();
}

void AuthenFriend::SlotApplyCancel()
{
    this->hide();
    deleteLater();
}
