#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include"global.h"
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();



private slots:
    void on_get_code_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_yes_pbt_clicked();



    void on_return_btn_clicked();

    void on_no_pbt_clicked();

private:
    void initHttpHandlers();
    void showTip(QString str,bool state);


    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVarifyValid();


    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);

    QMap<TipErr, QString>m_tip_errs;


    QTimer* _countdown_timer;
    int _countdown;

    void ChangeTipPage();

    Ui::RegisterDialog *ui;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;//这个回调函数是什么时候执行的????
    //答案:slot_reg_mod_finish中执行以ReqId为键值的处理逻辑函数,这个函数是在initHandler中注册的

signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
