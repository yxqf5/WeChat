#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();

private slots:
    void on_enter_pushButton_clicked();

    void on_return_pushButton_clicked();

signals:
    void sig_return_chatdlg();

private:
    Ui::ApplyFriend *ui;
};

#endif // APPLYFRIEND_H
