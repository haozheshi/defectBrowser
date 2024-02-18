#ifndef DEFECTPAGEMODEL_H
#define DEFECTPAGEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "defectfilterbook.h"


class defectPage:public QAbstractListModel
{
Q_OBJECT
public:
    enum Roles {
        imageRole = Qt::UserRole + 1,
        remarkRole = Qt::UserRole + 2,
        fileRole = Qt::UserRole + 3,
        rectRole = Qt::UserRole + 4,
        timeRole = Qt::UserRole + 5,
        countRole = Qt::UserRole + 6,
        majorRole = Qt::UserRole + 7
    };

    defectPage(defectFilterBook* book, QObject* parent = 0);
    ~defectPage();
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
public slots:
    void updateWords();
private:
    bool isIndexValid(const QModelIndex& index) const;
private:
    QList<queryresult> m_words;
    defectFilterBook* m_book = nullptr;
};

#endif // DEFECTPAGEMODEL_H
