#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include"global.h"
#include "StateWidget.h"
#include "applyfriend.h"
#include "userdata.h"
#include <QListWidgetItem>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void ShowSearch(bool bsearch = false);
    void addChatUserList();
    void AddLBGroup(StateWidget * lb);
    void ClearLabelState(StateWidget *lb);


    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata);



protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;
    void handleGlobalMousePress(QMouseEvent *event) ;

private:
    Ui::ChatDialog *ui;
    ChatUIMode m_mode;
    ChatUIMode m_state;
    bool m_b_loading;
        QWidget* _last_widget;

    QList<StateWidget*> m_lb_list;

    ApplyFriend *m_apply_friend_dlg;

    QMap<int, QListWidgetItem*> _chat_items_added;


    int _cur_chat_uid;

private slots:
    void slot_loading_chat_user();
    void slot_side_contact();
    void slot_side_chat();
    void slot_text_changed(const QString text);
    void slot_switch_apply_friend_page();
    void on_add_pbt_clicked();
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);


public slots:
    void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);

    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);


    void slot_return_chatdlg();
    void slot_chat_item_clicked(std::shared_ptr<UserInfo> theUserInfo);


signals:
    void sig_chat_item_clicked_from_chatdlg(std::shared_ptr<UserInfo> theUserInfo);


};

#endif // CHATDIALOG_H
