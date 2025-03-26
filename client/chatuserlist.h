#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H
#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include "chatuserwid.h"

class ChatUserList: public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    bool _load_pending;

private slots:
    void slot_item_clicked(QListWidgetItem* item);
signals:
    void sig_loading_chat_user();
    void sig_chat_item_clicked(std::shared_ptr<UserInfo> theUserInfo);
};

#endif // CHATUSERLIST_H
