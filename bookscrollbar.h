#ifndef BOOKSCROLLBAR_H
#define BOOKSCROLLBAR_H
#include <QScrollBar>
class bookScrollBar:public QScrollBar
{
 Q_OBJECT
 public:
   bookScrollBar(Qt::Orientation orientation, QWidget *parent);
   virtual void wheelEvent(QWheelEvent *event) override;
   void resetFlip();
signals:
   void FlipPage(bool flip);
private:
   bool bFlip = false;
};


#endif // BOOKSCROLLBAR_H
