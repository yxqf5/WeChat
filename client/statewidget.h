#ifndef STATEWIDGET_H
#define STATEWIDGET_H
#include <QWidget>
#include "global.h"
#include <QLabel>

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget* parent = nullptr);

public:
    void SetStates(QString normal,QString hover,QString press, QString select, QString select_hover, QString select_press);
    ClickLbState GetCurState();


    void ClearState();

    void SetSelected(bool bsetselected);

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

    virtual void paintEvent(QPaintEvent *event) override;


    void AddRedPoint();
    void ShowRedPoint(bool);


private:

    QLabel *m_red_point;


    QString m_normal;
    QString m_normal_hover;
    QString m_normal_press;

    QString m_select;
    QString m_select_hover;
    QString m_select_press;

    bool m_selected;

    ClickLbState m_curstate;


signals:
    void clicked();


};

#endif // STATEWIDGET_H
