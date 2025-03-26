#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H
#include<QLineEdit>
#include<QDebug>
#include<QWidget>
class CustomizeEdit :public QLineEdit
{
    Q_OBJECT
public:

    CustomizeEdit(QWidget* parent = nullptr);
    void SetMaxLength(int maxLen);

protected:
    void focusOutEvent(QFocusEvent * event)override;

    void limitTextLength(QString text);

private:
    int m_max_len;

signals:
    void sig_foucus_out();

};

#endif // CUSTOMIZEEDIT_H
