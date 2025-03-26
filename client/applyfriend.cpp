#include "applyfriend.h"
#include "ui_applyfriend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "tcpmgr.h"
#include "usermgr.h"

ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriend)
{
    ui->setupUi(this);
}

ApplyFriend::~ApplyFriend()
{
    delete ui;
}

void ApplyFriend::on_enter_pushButton_clicked()
{
    QJsonObject jsonstr;
    jsonstr["toname"] = ui->con_name_lineEdit->text();
    jsonstr["toemail"] = ui->email_lineEdit-> text();
    jsonstr["nickname"] = ui->nick_name_lineEdit->text();
    jsonstr["myuid"] = UserMgr::GetInstance()->GetUid();

    QJsonDocument doc(jsonstr);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug()<<" +++++++++++++++++++++++++++++++++++++++++++++++++ ";
    qDebug()<<" the apply_add_friend is: " << jsonstr;

    qDebug()<<" +++++++++++++++++++++++++++++++++++++++++++++++++ ";

    //调用连接发送请求数据
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ, jsonData);


}


void ApplyFriend::on_return_pushButton_clicked()
{

    emit sig_return_chatdlg();
}

