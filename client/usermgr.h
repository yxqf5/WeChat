#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <singleton.h>
#include "userdata.h"
#include <vector>

class UserMgr:public QObject,public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetName(QString name);
    void SetUid(int uid);
    void SetToken(QString token);
     std::shared_ptr<UserInfo> GetUserInfo();
    int GetUid();
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);


    bool AlreadyApply(int uid);
    void AddApplyList(std::shared_ptr<ApplyInfo> app);
   std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();

    bool CheckFriendById(int uid);
   void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
   void AddFriend(std::shared_ptr<AuthInfo> auth_info);
   std::shared_ptr<FriendInfo> GetFriendById(int uid);
   void AppendFriendChatMsg(int friend_id,std::vector<std::shared_ptr<TextChatData>>);


private:
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;

    //好友map
  QMap<int, std::shared_ptr<FriendInfo>> _friend_map;

    std::shared_ptr<UserInfo> _user_info;
    UserMgr();
    QString _name;
    QString _token;
    int _uid;
};

#endif // USERMGR_H
