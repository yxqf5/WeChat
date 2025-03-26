#include "chatuserlist.h"
#include<QScrollBar>
#include "usermgr.h"
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

ChatUserList::ChatUserList(QWidget *parent):QListWidget(parent), _load_pending(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);


    connect(this, &QListWidget::itemClicked, this, &ChatUserList::slot_item_clicked);


}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 0) {
            // auto b_loaded = UserMgr::GetInstance()->IsLoadChatFin();
            // if(b_loaded){
            //     return true;
            // }

            // if(_load_pending){
            //     return true;
            // }
            // 滚动到底部，加载新的联系人
            qDebug()<<"load more chat user";
            _load_pending = true;

            // QTimer::singleShot(100, [this](){
            //     _load_pending = false;
            //     QCoreApplication::quit(); // 完成后退出应用程序
            // });
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_chat_user();
        }

        return true; // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}

void ChatUserList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item);

    ChatUserWid* userwid = qobject_cast<ChatUserWid*>(widget);
    auto theUserInfo = userwid->ReturnUserInfo();
    qDebug()<<"the user is" <<theUserInfo->_name;

    emit sig_chat_item_clicked(theUserInfo);


}

// void ContactUserList::slot_item_clicked(QListWidgetItem *item)
// {
//     QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
//     if(!widget){
//         qDebug()<< "slot item clicked widget is nullptr";
//         return;
//     }

//     // 对自定义widget进行操作， 将item 转化为基类ListItemBase
//     ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
//     if(!customItem){
//         qDebug()<< "slot item clicked widget is nullptr";
//         return;
//     }

//     auto itemType = customItem->GetItemType();
//     if(itemType == ListItemType::INVALID_ITEM
//         || itemType == ListItemType::GROUP_TIP_ITEM){
//         qDebug()<< "slot invalid item clicked ";
//         return;
//     }

//     if(itemType == ListItemType::APPLY_FRIEND_ITEM){

//         // 创建对话框，提示用户
//         qDebug()<< "apply friend item clicked ";
//         //跳转到好友申请界面
//         emit sig_switch_apply_friend_page();
//         return;
//     }

//     if(itemType == ListItemType::CONTACT_USER_ITEM){
//         // 创建对话框，提示用户
//         qDebug()<< "contact user item clicked ";
//         //跳转到好友申请界面
//         emit sig_switch_friend_info_page();
//         return;
//     }
// }

