#ifndef PAGEITEMPICTURE_H
#define PAGEITEMPICTURE_H

#include <QObject>
#include <QIdentityProxyModel>
#include <QPixmap>
#include <QHash>
class pageItemPicture:public QIdentityProxyModel
{
public:
    pageItemPicture(QObject* parent = 0);
    QVariant data(const QModelIndex& index, int role) const override;
    void setSourceModel(QAbstractItemModel* sourceModel) override;
    void reloadPictures(const QModelIndex& startIndex, int count);
private:
   QHash<int, QPixmap*> m_pictures;
};

#endif // PAGEITEMPICTURE_H
