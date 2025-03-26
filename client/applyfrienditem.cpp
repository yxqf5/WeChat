#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"


ApplyFriendItem::ApplyFriendItem(QWidget *parent) :
    ListItemBase(parent), _added(false),
    ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::APPLY_FRIEND_ITEM);
    ui->addbtn->SetStatus("normal","hover", "press");
    ui->addbtn->hide();
    connect(ui->addbtn, &ClickedBtn::clicked,  [this](){
        emit this->sig_auth_friend(_apply_info);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;
    // 加载图片
    QPixmap pixmap(_apply_info->_icon);

    // 设置图片自动缩放
    ui->icon_label->setPixmap(pixmap.scaled(ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(_apply_info->_name);
    ui->user_chat_label_2->setText(_apply_info->_desc);
}

void ApplyFriendItem::Showaddbtn(bool bshow)
{
    if (bshow) {
        ui->addbtn->show();
        ui->already_label->hide();
        _added = false;
    }
    else {
        ui->addbtn->hide();
        ui->already_label->show();
        _added = true;
    }
}

int ApplyFriendItem::GetUid() {
    return _apply_info->_uid;
}


