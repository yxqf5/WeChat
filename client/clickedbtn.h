#ifndef CLICKEDBTN_H
#define CLICKEDBTN_H
#include<QPushButton>
#include"global.h"

class ClickedBtn :public QPushButton
{
    Q_OBJECT
public:
    ClickedBtn(QWidget * parent = nullptr);
    virtual ~ClickedBtn();

    virtual void enterEvent(QEnterEvent *event) override;//鼠标进入
    virtual void leaveEvent(QEvent *event) override;//鼠标离开
    virtual void mousePressEvent(QMouseEvent *e) override;//鼠标点击
    virtual void mouseReleaseEvent(QMouseEvent *e) override;//鼠标点击释放

    void SetStatus(QString m_hover,QString m_press,QString m_normal);

private:
    QString m_hover;
    QString m_press;
    QString m_normal;


};

#endif // CLICKEDBTN_H
