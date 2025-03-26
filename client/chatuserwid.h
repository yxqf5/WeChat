#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include <memory>

#include "listitembase.h"

#include "userdata.h"


namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }

    void SetInfo(QString name, QString head, QString msg);
    void SetInfo(std::shared_ptr<UserInfo> user_info);
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
        void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);

     std::shared_ptr<UserInfo> ReturnUserInfo(){
        return _user_info;
    }
private:
    Ui::ChatUserWid *ui;
    QString _name;
    QString _head;
    QString _msg;

    std::shared_ptr<UserInfo> _user_info;
    std::shared_ptr<UserInfo*> my_user_info;


};

#endif // CHATUSERWID_H
