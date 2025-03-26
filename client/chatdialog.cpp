#include "chatdialog.h"
#include "ui_chatdialog.h"
#include<QRandomGenerator>
#include"chatuserwid.h"
#include"loadingdlg.h"
#include <QElapsedTimer>
#include "statewidget.h""
#include "tcpmgr.h"
#include "usermgr.h"
#include "contactuserlist.h""



ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog),m_b_loading(false),m_mode(ChatUIMode::ChatMode),m_state(ChatUIMode::ChatMode)
{
    ui->setupUi(this);
    ui->add_pbt->SetStatus("normal","hover","press");

    ui->seach_lineEdit->setMaxLength(20);
    //清除图标功能
    // ui->seach_lineEdit->setClearButtonEnabled(true);
    QAction *searchAction = new QAction(ui->seach_lineEdit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->seach_lineEdit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->seach_lineEdit->setPlaceholderText("搜索");

    QAction *clearAction = new QAction(ui->seach_lineEdit);
    clearAction->setIcon(QIcon("/res/close_transparent.png"));
    ui->seach_lineEdit->addAction(clearAction, QLineEdit::TrailingPosition);

    connect(ui->seach_lineEdit, &QLineEdit::textChanged,[clearAction](const QString &text){
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }
    });

    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->seach_lineEdit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        ui->seach_lineEdit->clearFocus();
        //清除按钮被按下则不显示搜索框
        ShowSearch(false);
    });

    ShowSearch(false);


    //加载更多用户item
    connect(ui->chat_user_listWidget, &ChatUserList::sig_loading_chat_user,
            this, &ChatDialog::slot_loading_chat_user);

    addChatUserList();


    QPixmap pixmap(":/res/head_1.jpg");
    ui->user_icon_label->setPixmap(pixmap);

    QPixmap scaledPixmap = pixmap.scaled( ui->user_icon_label->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    ui->user_icon_label->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->user_icon_label->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    ui->chat_user_widget_2->setProperty("state","normal");

    ui->chat_user_widget_2->SetStates("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    ui->con_user_widget->SetStates("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    // AddLBGroup(ui->chat_user_widget_2);
    // AddLBGroup(ui->con_user_widget);

    connect(ui->chat_user_widget_2, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->con_user_widget, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);


    connect(ui->seach_lineEdit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);

    //检测鼠标点击位置判断是否要清空搜索框
    this->installEventFilter(this); // 安装事件过滤器

    //连接联系人页面点击好友申请条目的信号
    connect(ui->con_user_listWidget, &ContactUserList::sig_switch_apply_friend_page,
            this,&ChatDialog::slot_switch_apply_friend_page);


    //好友申请信号处理
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_apply_friend);

    //连接认证添加好友信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);

    //链接自己认证回复信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this,
            &ChatDialog::slot_auth_rsp);


    //点击chatUserItem后,将当前的聊天uid进行设定;
    connect(ui->chat_user_listWidget, &ChatUserList::sig_chat_item_clicked, this, &ChatDialog::slot_chat_item_clicked);

    //    //连接收到消息后的信号处理工作
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);

}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::ShowSearch(bool bsearch)
{
    if(bsearch){
        ui->chat_user_listWidget->hide();
        ui->con_user_listWidget->hide();
        ui->search_listWidget->show();
        m_mode = ChatUIMode::SearchMode;
    }
    else if(m_state == ChatUIMode::ChatMode){
        ui->chat_user_listWidget->show();
        ui->con_user_listWidget->hide();
        ui->search_listWidget->hide();
        m_mode = ChatUIMode::ChatMode;

        ui->seach_lineEdit->clear();

    }
    else if(m_state == ChatUIMode::ContactMode){
        ui->chat_user_listWidget->hide();
        ui->con_user_listWidget->show();
        ui->search_listWidget->hide();
        m_mode = ChatUIMode::ChatMode;

        ui->seach_lineEdit->clear();

    }
}

void ChatDialog::UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata)
{
    for(auto & msg : msgdata){
        if(msg->_from_uid != _cur_chat_uid){
            break;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}


void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    // for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_listWidget->addItem(item);
        ui->chat_user_listWidget->setItemWidget(item, chat_user_wid);
    // }
}

void ChatDialog::AddLBGroup(StateWidget *lb)
{
    m_lb_list.push_back(lb);
}

void ChatDialog::slot_loading_chat_user()
{
    if(m_b_loading){
        return;
    }

    m_b_loading = true;

    //LoadingDlg就是显示一个加载的界面,但是速度比较快,所以去掉也没事
    // LoadingDlg *loadingDialog = new LoadingDlg(this);
    // loadingDialog->setModal(true);
    // loadingDialog->show();

    qDebug() << "add new data to list.....";

    addChatUserList();
    // 加载完成后关闭对话框



    // QElapsedTimer t;

    // t.start();

    // while(t.elapsed()<10000);
    // loadingDialog->deleteLater();

    m_b_loading = false;

}
void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto & ele: m_lb_list){
        if(ele == lb){
            continue;
        }

        ele->ClearState();
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched,event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    if(m_mode != ChatUIMode::SearchMode){
        return;
    }


    QPoint posInSearchList = ui->search_listWidget->mapFromGlobal(event->globalPos());

    if(false == ui->search_listWidget->rect().contains(posInSearchList)){
        ui->seach_lineEdit->clear();
        ShowSearch(false);
    }

}

void ChatDialog::slot_side_chat()
{
    qDebug()<<"receive side chat clicked";

    ClearLabelState(ui->chat_user_widget_2);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    m_state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_text_changed(const QString text)
{
    if(text.size()){
        ShowSearch(true);
    }

    // ShowSearch(false);
}

void ChatDialog::slot_side_contact()
{
    qDebug()<< "receive side contact clicked";
    ClearLabelState(ui->con_user_widget);
    //设置
    if(_last_widget == nullptr){
        // ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
        // _last_widget = ui->friend_apply_page;
    }else{
        ui->stackedWidget->setCurrentWidget(_last_widget);
    }

    m_state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_switch_apply_friend_page()
{
    qDebug()<<"receive switch apply friend page sig";
    _last_widget = ui->apply_friend_page;
    ui->stackedWidget->setCurrentWidget(ui->apply_friend_page);
}


void ChatDialog::on_add_pbt_clicked()
{
    m_apply_friend_dlg = new ApplyFriend(this);

    m_apply_friend_dlg->show();

    //处理好友添加界面的返回功能
    connect(m_apply_friend_dlg, &ApplyFriend::sig_return_chatdlg, this, &ChatDialog::slot_return_chatdlg);

}

void ChatDialog::slot_apply_friend(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid << " name is "
             << apply->_name << " desc is " << apply->_desc;

    bool b_already = UserMgr::GetInstance()->AlreadyApply(apply->_from_uid);
    if(b_already){
        return;
    }

    UserMgr::GetInstance()->AddApplyList(std::make_shared<ApplyInfo>(apply));
    ui->con_user_widget->ShowRedPoint(true);
    ui->con_user_listWidget->ShowRedPoint(true);//新朋友显示红点
    ui->apply_friend_page->AddNewApply(apply);


    //这里可能需要添加uid和item给到map中,用于消掉添加按钮

}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info) {
    qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
             << " name is " << auth_info->_name << " nick is " << auth_info->_nick;

    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if(bfriend){
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_info);

    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_listWidget->insertItem(0, item);
    ui->chat_user_listWidget->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->_uid, item);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid
             << " name is " << auth_rsp->_name << " nick is " << auth_rsp->_nick;

    //判断如果已经是好友则跳过
    // auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    // if(bfriend){
    //     return;
    // }

    UserMgr::GetInstance()->AddFriend(auth_rsp);
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_listWidget->insertItem(0, item);
    ui->chat_user_listWidget->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}



void ChatDialog::slot_return_chatdlg()
{
        m_apply_friend_dlg->hide();
}

void ChatDialog::slot_chat_item_clicked(std::shared_ptr<UserInfo> theUserInfo)
{
    qDebug()<<"[Before change] the _cur_chat_uid is "<<_cur_chat_uid;

    this->_cur_chat_uid = theUserInfo->_uid;


    qDebug()<<"[After change] the _cur_chat_uid is "<<_cur_chat_uid;

    ui->chat_page->setCurrentUserInfo(theUserInfo);

}


void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg)
{
    auto find_iter = _chat_items_added.find(msg->_from_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "set chat item msg, uid is " << msg->_from_uid;
        QWidget *widget = ui->chat_user_listWidget->itemWidget(find_iter.value());
        auto chat_wid = qobject_cast<ChatUserWid*>(widget);
        if(!chat_wid){
            return;
        }
        chat_wid->updateLastMsg(msg->_chat_msgs);
        //更新当前聊天页面记录
        UpdateChatMsg(msg->_chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
        return;
    }

    //如果没找到，则创建新的插入listwidget

    auto* chat_user_wid = new ChatUserWid();
    //查询好友信息
    auto fi_ptr = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
    chat_user_wid->SetInfo(fi_ptr);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    chat_user_wid->updateLastMsg(msg->_chat_msgs);
    UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
    ui->chat_user_listWidget->insertItem(0, item);
    ui->chat_user_listWidget->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->_from_uid, item);

}
