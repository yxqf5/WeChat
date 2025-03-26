#include "timerbtn.h"

#include<QMouseEvent>
#include<QDebug>

TimerBtn::TimerBtn(QWidget *parent) :QPushButton(parent), m_counter(10)
{
    m_timer = new QTimer(this);

    connect(m_timer, &QTimer::timeout, [this](){

        m_counter--;

        if(m_counter <= 0){
            m_timer->stop();
            m_counter = 10;

            this->setText("获取");
            this->setEnabled(true);
            return;

        }

        this->setText(QString::number(m_counter));

    });

}

TimerBtn::~TimerBtn()
{
    m_timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){

        qDebug()<< "MyButton was released!" ;

        this->setEnabled(false);
        this->setText(QString::number(m_counter));
        m_timer->start(1000);
        emit clicked();
    }

    QPushButton::mouseReleaseEvent(e);
}
