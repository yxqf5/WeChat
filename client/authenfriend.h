#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include <QDialog>
#include "global.h"
#include "usermgr.h"
#include "tcpmgr.h"

namespace Ui {
class AuthenFriend;
}

class AuthenFriend : public QDialog
{
    Q_OBJECT

public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();
public slots:
    void SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info);

        //处理确认回调
    void SlotApplySure();
    //处理取消回调
    void SlotApplyCancel();



private:

    std::shared_ptr<ApplyInfo> _apply_info;

    Ui::AuthenFriend *ui;
};

#endif // AUTHENFRIEND_H
