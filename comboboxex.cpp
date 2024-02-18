#include "comboboxex.h"
#include <QCheckBox>
#include <QPalette>
#include <QListWidgetItem>
comboBoxEx::comboBoxEx(QWidget *parent):
    QComboBox(parent)
{
    m_listWidget = new QListWidget(parent);
    setModel(m_listWidget->model());
    setView(m_listWidget);
}

void comboBoxEx::addItem(QString text)
{
    QCheckBox *checkBox = new QCheckBox(m_listWidget);
    checkBox->setFixedWidth(30);
    QListWidgetItem *item = new QListWidgetItem;
    m_listWidget->addItem(item);
    item->setText(text);
    item->setTextAlignment(Qt::AlignRight);
    m_listWidget->setItemWidget(item, checkBox);
}

QStringList comboBoxEx::getSelected()
{
    QStringList list;
    int count = m_listWidget->count();
    for(int i = 0; i < count; i++)
    {
        QListWidgetItem* item = m_listWidget->item(i);
        QCheckBox *checkbox = static_cast<QCheckBox *>(m_listWidget->itemWidget(item));
        if(checkbox->isChecked())
        {
            list << item->text();
        }
    }
    return list;
}
