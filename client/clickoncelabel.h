#ifndef CLICKONCELABEL_H
#define CLICKONCELABEL_H

#include <QLabel>


class ClickedOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedOnceLabel(QWidget *parent=nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
signals:
    void clicked(QString);
};
#endif // CLICKONCELABEL_H
