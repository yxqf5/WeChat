#include "statewidget.h"
#include <QVBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>

StateWidget::StateWidget(QWidget *parent): QWidget(parent),m_curstate(ClickLbState::Normal)
{

    //鼠标变小手
    setCursor(Qt::PointingHandCursor);

    AddRedPoint();


}

void StateWidget::SetStates(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
     m_normal = normal;
     m_normal_hover = hover;
     m_normal_press = press;

     m_select = select;
     m_select_hover = select_hover;
     m_select_press = select_press;

     setProperty("state", normal);
     repolish(this);
}

ClickLbState StateWidget::GetCurState()
{
    return m_curstate;
}

void StateWidget::ClearState()
{
    m_curstate = ClickLbState::Normal;
    setProperty("state", m_normal);
    repolish(this);
    update();
}

void StateWidget::SetSelected(bool bsetselected)
{
    if(bsetselected){
        m_curstate = ClickLbState::Selected;
        setProperty("state", m_select);
        repolish(this);
        update();
        return;
    }

    m_curstate = ClickLbState::Selected;
    setProperty("state", m_normal);
    repolish(this);
    update();
    return;

}


// 处理鼠标点击事件
void StateWidget::mousePressEvent(QMouseEvent* event)  {
    if (event->button() == Qt::LeftButton) {
        if(m_curstate == ClickLbState::Selected){
            qDebug()<<"PressEvent , already to selected press: "<< m_select_press;
            //emit clicked();
            // 调用基类的mousePressEvent以保证正常的事件处理
            QWidget::mousePressEvent(event);
            return;
        }

        if(m_curstate == ClickLbState::Normal){
            qDebug()<<"PressEvent , change to selected press: "<<m_select_press;
            m_curstate = ClickLbState::Selected;
            setProperty("state",m_select_press);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(m_curstate == ClickLbState::Normal){
            qDebug()<<"ReleaseEvent , change to normal hover: "<< m_normal_hover;
            setProperty("state",m_normal_hover);
            repolish(this);
            update();

        }else{
            qDebug()<<"ReleaseEvent , change to select hover: "<< m_select_hover;
            setProperty("state",m_select_hover);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void StateWidget::enterEvent(QEnterEvent* event) {
    // 在这里处理鼠标悬停进入的逻辑
    if(m_curstate == ClickLbState::Normal){
        //qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state",m_normal_hover);
        repolish(this);
        update();

    }else{
        //qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state",m_select_hover);
        repolish(this);
        update();
    }

    QWidget::enterEvent(event);
}



// 处理鼠标悬停离开事件
void StateWidget::leaveEvent(QEvent* event){
    // 在这里处理鼠标悬停离开的逻辑
    if(m_curstate == ClickLbState::Normal){
        // qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",m_normal);
        repolish(this);
        update();

    }else{
        // qDebug()<<"leave , change to select normal : "<< _selected;
        setProperty("state",m_selected);
        repolish(this);
        update();
    }
    QWidget::leaveEvent(event);
}




void StateWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt, &p, this);

}



//先生成一个label,然后放到一个垂直布局里,然后将垂直布局放到类对象中
void StateWidget::AddRedPoint()
{

    m_red_point = new QLabel();

    m_red_point->setObjectName("red_point");
    QVBoxLayout* layout2 = new QVBoxLayout;

    m_red_point->setAlignment(Qt::AlignCenter);

    layout2->addWidget(m_red_point);
    layout2->setContentsMargins(0,0,0,0);

    this->setLayout(layout2);
    //先隐藏
    m_red_point->setVisible(false);


}

void StateWidget::ShowRedPoint(bool)
{
    m_red_point->setVisible(true);
}
