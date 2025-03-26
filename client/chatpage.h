#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include"clickedlabel.h"
#include"clickedbtn.h"
#include "chatview.h"
#include "userdata.h"

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void paintEvent(QPaintEvent *event);

    void AppendChatMsg(std::shared_ptr<TextChatData> msg);
    void setCurrentUserInfo(std::shared_ptr<UserInfo> theUserInfo){
        _user_info = theUserInfo;
    }
private slots:
    void on_send_btn_clicked();

    void on_receive_btn_clicked();

signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);


private:
    Ui::ChatPage *ui;

    std::shared_ptr<UserInfo> _user_info;

    QMap<QString, QWidget*>  _bubble_map;

};

#endif // CHATPAGE_H
