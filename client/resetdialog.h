#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include<global.h>

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

private slots:



    void on_code_btn_clicked();

    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_enter_btn_clicked();

    void on_return_btn_clicked();

signals:
    void switchLogin();


private:
    void showTip(QString str, bool b_ok);

    void initHandlers();
    bool checkUserValid();
    bool checkPassValid();
    bool checkEmailValid();
    bool checkVarifyValid();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);

    QMap<TipErr, QString>_tip_errs;
    Ui::ResetDialog *ui;
    QMap<ReqId,std::function<void(const QJsonObject&)>>_handlers;
};

#endif // RESETDIALOG_H
