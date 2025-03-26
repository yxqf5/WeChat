#include "clickedbtn.h"



ClickedBtn::ClickedBtn(QWidget *parent) : QPushButton(parent)
{

}

ClickedBtn::~ClickedBtn(){

}




void ClickedBtn::enterEvent(QEnterEvent *event)
{
    setProperty("state",m_hover);
    repolish(this);
    update();
    QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent *event)
{
    setProperty("state",m_normal);
    repolish(this);
    update();
    QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent *e)
{
    setProperty("state",m_press);
    repolish(this);
    update();
    QPushButton::mousePressEvent(e);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent *e)
{
    setProperty("state",m_hover);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(e);
}

void ClickedBtn::SetStatus(QString normal , QString hover, QString press)
{
    m_press = press;
    m_hover = hover;
    m_normal = normal;
    setProperty("state",normal);
    repolish(this);
    update();
}
