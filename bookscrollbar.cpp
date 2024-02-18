#include "bookscrollbar.h"
#include <QDebug>
#include <QWheelEvent>

bookScrollBar::bookScrollBar(Qt::Orientation orientation, QWidget *parent):
    QScrollBar(orientation, parent)
{

}

void bookScrollBar::wheelEvent(QWheelEvent *event)
{
    if(!bFlip)
    {
        if(value() == maximum() && event->delta() < 0)
        {
            bFlip = true;
            emit FlipPage(0);
            return;
        }
        else if(value() == minimum() && event->delta() > 0)
        {
            bFlip = true;
            emit FlipPage(1);
            return;
        }
        QScrollBar::wheelEvent(event);
    }
}

void bookScrollBar::resetFlip()
{
    bFlip = false;
}

