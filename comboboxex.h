#ifndef COMBOBOXEX_H
#define COMBOBOXEX_H

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
class comboBoxEx:public QComboBox
{
public:
    comboBoxEx(QWidget *parent = nullptr);
    void addItem(QString text);
    QStringList getSelected();
private:
    QListWidget* m_listWidget;
};

#endif // COMBOBOXEX_H
