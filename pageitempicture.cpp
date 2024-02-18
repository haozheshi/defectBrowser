#include "defectpagepage.h"
#include "pageitempicture.h"

pageItemPicture::pageItemPicture(QObject* parent):
     QIdentityProxyModel(parent),
     m_pictures()
{

}

QVariant pageItemPicture::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DecorationRole) {
        return QIdentityProxyModel::data(index, role);
    }
    return *m_pictures[index.row()];
}

void pageItemPicture::setSourceModel(QAbstractItemModel* sourceModel)
{
    QIdentityProxyModel::setSourceModel(sourceModel);
    if (!sourceModel)
    {
        return;
    }

    connect(sourceModel, &QAbstractItemModel::modelReset, [this] {
         reloadPictures(index(0, 0), rowCount());
     });
}

void pageItemPicture::reloadPictures(const QModelIndex& startIndex, int count)
{
    const QAbstractItemModel* model = startIndex.model();
    int lastIndex = startIndex.row() + count;
    for(int row = startIndex.row(); row < lastIndex; row++) {
        QByteArray qba = model->data(model->index(row, 0), defectPage::Roles::imageRole).toByteArray();
        if(qba.size() > 0)
        {
            QImage im = QImage::fromData((const uchar*)qba.data(), qba.size(), "jpg");
            QPixmap pixmap = QPixmap::fromImage(im);
            auto thumbnail = new QPixmap(pixmap
                                         .scaled(196, 196,
                                                 Qt::KeepAspectRatio,
                                                 Qt::FastTransformation));
            m_pictures.insert(row, thumbnail);
        }
        else
        {
             QImage im(196, 196, QImage::Format_BGR888);
             im.fill(QColor(125,125,125));
             QPixmap pixmap = QPixmap::fromImage(im);
             auto thumbnail = new QPixmap(pixmap
                                          .scaled(196, 196,
                                                  Qt::KeepAspectRatio,
                                                  Qt::FastTransformation));
             m_pictures.insert(row, thumbnail);
        }
    }
}
