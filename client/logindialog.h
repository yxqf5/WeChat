#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include"global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    int _uid;
    QString _token;



private:
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;

    void initHttpHandlers();
    void showTip(QString,bool);

    bool checkUserValid();
    bool checkPasswordValid();

    bool enableBtn(bool enabled);

    Ui::LoginDialog *ui;

private slots:
   void  slot_resetpass();

    void on_login_btn_clicked();
   void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);

    void slot_con_finish(bool bsuccess);

   void slot_login_failed(int err);

signals:
    void switchRegister();
    void switchReset();

    void sig_connect_tcp(ServerInfo si);

};

#endif // LOGINDIALOG_H
