#include "PictureDelegate.h"
#include "defectpagepage.h"
#include <QPainter>

const unsigned int BANNER_HEIGHT = 20;
const unsigned int BANNER_COLOR = 0x303080;
const unsigned int BOT_COLOR = 0xffffff;
const unsigned int BANNER_ALPHA = 200;
const unsigned int BANNER_TEXT_COLOR = 0xffffff;
const unsigned int Bot_TEXT_COLOR = 0x000000;
const unsigned int HIGHLIGHT_ALPHA = 50;

PictureDelegate::PictureDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

void PictureDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    QPixmap pixmap = index.model()->data(index, Qt::DecorationRole).value<QPixmap>();
    painter->drawPixmap(option.rect.x(), option.rect.y() + 20, pixmap);
    QFont font;
    font.setPixelSize(16);
    painter->setFont(font);
    //标号
    QRect indexRect = QRect(option.rect.x(), option.rect.y(), 60, BANNER_HEIGHT);
    QColor indexColor;
    bool judge = index.model()->data(index, defectPage::majorRole).toBool();
    if(judge)
    {
       indexColor =  QColor(0xff0000);
    }
    else
    {
       indexColor = QColor(0xffff00);
    }
    indexColor.setAlpha(BANNER_ALPHA);
    painter->fillRect(indexRect, indexColor);
    QString count = index.model()->data(index, defectPage::countRole).toString();
    painter->drawText(indexRect, Qt::AlignRight, count);
    //title
    QRect bannerRect = QRect(option.rect.x() + 60, option.rect.y(), pixmap.width()-60, BANNER_HEIGHT);
    QColor bannerColor = QColor(BANNER_COLOR);
    bannerColor.setAlpha(BANNER_ALPHA);
    painter->fillRect(bannerRect, bannerColor);
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    painter->setPen(BANNER_TEXT_COLOR);
    painter->drawText(bannerRect, Qt::AlignRight, title);

    //底部
    QRect botRect = QRect(option.rect.x(), option.rect.y() +
                          pixmap.height() + BANNER_HEIGHT, pixmap.width(), BANNER_HEIGHT);
    bannerColor.setAlpha(BANNER_ALPHA);
    painter->fillRect(botRect, BOT_COLOR);
    QString remark = index.model()->data(index, defectPage::timeRole).toString();
    painter->setPen(Bot_TEXT_COLOR);
    painter->drawText(botRect, Qt::AlignCenter, remark);

    if (option.state.testFlag(QStyle::State_Selected)) {
        QColor selectedColor = option.palette.highlight().color();
        selectedColor.setAlpha(HIGHLIGHT_ALPHA);
        painter->fillRect(option.rect, selectedColor);
    }
    painter->restore();
}

QSize PictureDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    const QPixmap& pixmap = index.model()->data(index, Qt::DecorationRole).value<QPixmap>();
    QSize sz = pixmap.size();
    sz.setHeight(sz.height() + 2 * BANNER_HEIGHT);
    return sz;
}
