#include "pcie.h"
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QScrollBar>
#include <QSettings>
#include <QGraphicsTextItem>
#include <QCoreApplication>


#define BTNSTYLE1(btn)  btn->setFlat(true); \
btn->setFixedSize(QSize(32,32));\

pcie::pcie(QWidget* parent):
    QWidget(parent),
    m_RtTracker(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    this->setLayout(layout);
    layout->setMargin(0);
    m_pScene = new QGraphicsScene(this);
    m_pScene->setSceneRect(QRectF(-INT_MAX / 2,-INT_MAX / 2,
                                  INT_MAX, INT_MAX));
    m_pView = new myView(this);
    m_curChooseGraphic = nullptr;
//    m_pView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_pView->setRenderHints(QPainter::Antialiasing);
    m_pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_pView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pView->setScene(m_pScene);
    m_BtnZoomFit = new QPushButton("fit", this);
    BTNSTYLE1(m_BtnZoomFit)
    QVBoxLayout* btnlayout = new QVBoxLayout();
    layout->addWidget(m_pView, 98);
    layout->addLayout(btnlayout);
    btnlayout->addStretch(98);
    btnlayout->addWidget(m_BtnZoomFit,2);
    //事件
    connect(m_BtnZoomFit, &QPushButton::clicked, this,
            &pcie::on_Fit);
    connect(m_pView, &myView::track, this, &pcie::on_UpdateTracker);
    connect(m_pView, &myView::beginTrack, this, &pcie::on_BeginTrack);
    connect(m_pView, &myView::endTrack, this, &pcie::on_endTrack);
    connect(m_pView, &myView::dbClicked, this, &pcie::on_DbClick);
    connect(m_pView, &myView::beginPolyon, this, &pcie::on_BeginPolyon);
    connect(m_pView, &myView::updatePolygon, this, &pcie::on_UpdatePolyon);
    connect(m_pView, &myView::endPolyon, this, &pcie::on_EndPolyon);
    connect(m_pView, &myView::trackPolyon, this, &pcie::trackPolyon);
    connect(m_pView, &myView::selectChanged, this, &pcie::selectedChange);
    connect(m_pView, &myView::lookAt, this, &pcie::getLookAt);
    connect(m_pView, &myView::viewCofChange, this,
             &pcie::viewCofChange);
}

bool pcie::setImage(QImage &image)
{
    image.detach();
    m_image = image.copy();
    QPixmap ConvertPixmap = QPixmap::fromImage(image);
    this->clearImage();
    m_RtTracker = nullptr;
    auto im = new QGraphicsPixmapItem(ConvertPixmap);
    im->setZValue(-1);
    im->setFlag(QGraphicsItem::ItemIsSelectable, false);
    m_pScene->addItem(im);
    return true;
}

void pcie::setRect(QRect rect, int r, int g, int b, bool fill)
{
    QGraphicsRectItem* newOpRect = new QGraphicsRectItem();
    newOpRect->setRect(rect);
    QPen pen(QColor(r,g,b));
    pen.setWidth(5);
    newOpRect->setPen(pen);
    m_pScene->addItem(newOpRect);
}
void pcie::setText(QString s, int x, int y, int size)
{
    QGraphicsSimpleTextItem* text = new QGraphicsSimpleTextItem();
    text->setFont(QFont("宋体",size));
    text->setPos(x, y);
    text->setText(s);
    text->setBrush(QBrush(QColor(255, 30, 30)));
    m_pScene->addItem(text);
}
void pcie::setVAERegion(QPoint p1, QPoint p2, int r,
                        int g, int b,
                        int type, std::string name)
{
    QRect rect = QRect(p1, p1).normalized();
    myRectRegion* newOpRect = new myRectRegion(rect, 0, true);
    newOpRect->setName(name);
    newOpRect->setPoints(p1, p2);
    newOpRect->m_type = type;
    QBrush brush(QColor(r, g, b, 160));
    newOpRect->setBrush(brush);
    QPen pen(QColor(r, g, b, 160));
    pen.setWidth(2);
    newOpRect->setPen(pen);
    m_pScene->addItem(newOpRect);
}

void pcie::setImageMask(QImage& qim, std::string name)
{
    qim.detach();
    QPixmap mm = QPixmap::fromImage(qim);
    myPixmapGraphics* maskm = new myPixmapGraphics(mm);
    maskm->setName(name);
    m_pScene->addItem(maskm);
}

void pcie::setPolygon(std::vector<float> &x, std::vector<float> &y,
                      int r, int g, int b, bool fill)
{
    QGraphicsPolygonItem* polygon = new QGraphicsPolygonItem();
    QPolygonF points;
    for(size_t i = 0; i < x.size(); i++)
    {
        points << QPointF(x[i], y[i]);
    }
    if(fill)
    {
        QBrush brush(QColor(r,g,b, 70));
        polygon->setBrush(brush);
    }
    else
    {
        QPen pen(QColor(r,g,b));
        pen.setWidth(5);
        polygon->setPen(pen);
    }
    polygon->setPolygon(points);
    m_pScene->addItem(polygon);
}

void pcie::setCircle(float x, float y, float ra,
                     int r, int g, int b, bool fill)
{
    QGraphicsEllipseItem* circle = new QGraphicsEllipseItem();
    if(fill)
    {
        QBrush brush(QColor(r,g,b, 70));
        circle->setBrush(brush);
    }
    else
    {
        QPen pen(QColor(r, g, b));
        circle->setPen(pen);
        pen.setWidth(8);
    }
    circle->setRect(QRectF(x - ra, y - ra, 2 * ra, 2 * ra));
    m_pScene->addItem(circle);
}

void pcie::setLine(float x1, float y1, float x2,
                   float y2, int r, int g, int b,
                   bool fill)
{
    Q_UNUSED(fill)
    QGraphicsLineItem* line = new QGraphicsLineItem();
    line->setLine(x1, y1, x2, y2);
    QPen pen(QColor(r,g,b, 255));
    pen.setWidth(8);
    line->setPen(pen);
    m_pScene->addItem(line);
}

void pcie::setRuler(float x1, float y1, float x2, float y2,
                    int r, int g, int b, bool fill)
{
    Q_UNUSED(fill)
    QGraphicsPathItem* ruler = new QGraphicsPathItem();
    QPainterPath path;
    path.moveTo(x1, y1);
    float dx = x2 - x1;
    float dy = y2 - y1;
    float L1 = sqrt(dx * dx + dy * dy + 1);
    dx = dx / L1;
    dy = dy / L1;
    float pdx = -dy;
    float pdy = dx;
    path.lineTo(x1 + 5 * (dx + pdx), y1 + 5 * (dy + pdy));
    path.moveTo(x1, y1);
    path.lineTo(x1 + 5 * (dx - pdx), y1 + 5 * (dy  -pdy));
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    path.lineTo(x2 - 5 * (dx + pdx), y2 - 5 * (dy + pdy));
    path.moveTo(x2, y2);
    path.lineTo(x2 - 5 * (dx - pdx), y2 - 5 * (dy  -pdy));
    ruler->setPath(path);
    QPen pen(QColor(r,g,b, 255));
    pen.setWidth(8);
    ruler->setPen(pen);
    m_pScene->addItem(ruler);
}

void pcie::cleargeo()
{
    auto ilst = m_pScene->items();
    for(auto item: ilst)
    {
        int t = item->type();
        item->setSelected(false);
        if(t != 7)
        {
            m_pScene->removeItem(item);
            delete item;
        }
    }
    m_RtTracker = nullptr;
}


void pcie::clearImage()
{
    auto ilst = m_pScene->items();
    for(auto item: ilst)
    {
        int t = item->type();
        item->setSelected(false);
        if(t == 7)
        {
            m_pScene->removeItem(item);
            delete item;
        }
    }
    m_RtTracker = nullptr;
}

void pcie::removeOneGeo(QGraphicsItem *item)
{
    m_pScene->removeItem(item);
    delete  item;
    m_curChooseGraphic = nullptr;
    m_pScene->update();
}

void pcie::setTrackerType(int type)
{
    m_ntype = type;
}

void pcie::on_Fit()
{
    m_pView->resetTransform();
    float m_w = m_pView->width();
    float  m_h = m_pView->height();
    float ratio_w = 1.0;
    float ratio_h = 1.0;
    QRectF urect(0, 0, 0, 0);
    for(auto itm: m_pScene->items()){
        QRectF rect = itm->boundingRect();
        urect |= rect;
    }
    m_pView->centerOn(urect.width() / 2, urect.height()/2);
    m_pView->view_x =urect.width() / 2;
    m_pView->view_y =urect.height()/2;
    ratio_w = m_w / float(urect.width() + 1) * 0.95;
    ratio_h = m_h / float(urect.height() + 1) * 0.95;
    if(ratio_w > ratio_h){
        m_pView->view_sx = ratio_h;
        m_pView->view_sy = ratio_h;
        m_pView->scale(ratio_h, ratio_h);
    }else{
        m_pView->view_sx = ratio_w;
        m_pView->view_sy = ratio_w;
        m_pView->scale(ratio_w, ratio_w);
    }
}

void pcie::setViewCOff(int x, int y, float scalex,
                 float scaley)
{
    m_pView->resetTransform();
    m_pView->centerOn(x, y);
    m_pView->scale(scalex, scaley);
    m_pView->view_x = x;
    m_pView->view_y = y;
    m_pView->view_sx = scalex;
    m_pView->view_sy = scaley;
}


void pcie::setMode(PCIEMod mode)
{
    m_pView->setMode(mode);
}

void pcie::on_BeginTrack()
{
    m_RtTracker = new myRectRegion(QRectF(0, 0, 0, 0), 0, true);
    QBrush brush(QColor(0, 0, 255, 160));
    m_RtTracker->setBrush(brush);
    QPen pen(QColor(0, 0, 255, 160));
    pen.setWidth(2);
    m_RtTracker->setPen(pen);
    m_pScene->addItem(m_RtTracker);
    m_RtTracker->setVisible(false);
    m_RtTracker->m_type = m_ntype;
}

void pcie::on_UpdateTracker(QPoint p1, QPoint p2)
{
    m_RtTracker->setVisible(true);
    m_RtTracker->setPoints(p1, p2);
    m_pScene->update();
}

void pcie::on_endTrack()
{
    emit rectDraw(m_RtTracker->m_p1, m_RtTracker->m_p2);
    if(m_RtTracker != nullptr)
    {
        m_RtTracker->setVisible(false);
        m_pScene->removeItem(m_RtTracker);
    }
}


void pcie::on_BeginPolyon(QPoint point)
{
    m_PolygonTracker = new QGraphicsPolygonItem();
    QBrush brush(QColor(0, 255, 0, 160));
    m_PolygonTracker->setBrush(brush);
    m_pScene->addItem(m_PolygonTracker);
    QPolygonF polygon = m_PolygonTracker->polygon();
    polygon.append(point);
    polygon.append(point);
    m_PolygonTracker->setPolygon(polygon);
    m_pScene->update();
}

void pcie:: trackPolyon(QPoint point)
{
    QPolygonF polygon = m_PolygonTracker->polygon();
    polygon.back().setX(point.x());
    polygon.back().setY(point.y());
    m_PolygonTracker->setPolygon(polygon);
    m_pScene->update();
}


void pcie::on_UpdatePolyon(QPoint point)
{
    QPolygonF polygon = m_PolygonTracker->polygon();
    polygon.back().setX(point.x());
    polygon.back().setY(point.y());
    polygon.append(point);
    m_PolygonTracker->setPolygon(polygon);
    m_pScene->update();
}

void pcie::on_EndPolyon(QPoint point)
{
    Q_UNUSED(point)
    QPolygonF polygonF = m_PolygonTracker->polygon();
    QList<QPoint> polygon;
    for(int i = 0; i < polygonF.size(); i++)
    {
        polygon << polygonF[i].toPoint();
    }
    m_pScene->removeItem(m_PolygonTracker);
    emit maskPolyon(polygon);
}

QGraphicsItem* pcie::getChoose(std::string& name)
{
    if(m_curChooseGraphic != nullptr)
    {
        name = m_curChooseGraphic->data(Qt::UserRole + 1).toString().toStdString();
    }
    return m_curChooseGraphic;
}

void pcie::selectedChange(QGraphicsItem* item)
{
    m_curChooseGraphic = item;
}

void pcie::on_DbClick(QPoint p)
{
    auto s = m_pView->getSelect(p);
    if(s != nullptr)
    {
        auto v = s->data(Qt::UserRole + 1).toString().toStdString();
        emit rectEdit(v);
    }
}


void myView::setMode(PCIEMod mode)
{
    m_mode = mode;
}

void myView::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
    {
        view_sx *= 1.1;
        view_sy *= 1.1;
        this->scale(1.1, 1.1);
    }
    else
    {
        view_sx *= 0.9;
        view_sy *= 0.9;
        this->scale(0.9, 0.9);
    }
    emit viewCofChange(view_x, view_y,
                       view_sx, view_sy);
}


void myView::mousePressEvent(QMouseEvent *event)
{
    // 绘制模式
    QPointF point = this->mapToScene(event->x(), event->y());
    if(m_mode == DRAW)
    {
        if(event->button() == Qt::LeftButton)
        {
            m_p1 = point.toPoint();
            m_p2 = point.toPoint();
            bTrack = true;
            emit beginTrack();
        }
    }
    else if(m_mode == POLYGON)
    {
        if(event->button() == Qt::LeftButton)
        {
            if(!m_bPolygonBegin)
            {
                m_bPolygonBegin = true;
                setMouseTracking(true);
                emit beginPolyon(point.toPoint());
            }
            else
            {
                emit updatePolygon(point.toPoint());
            }
        }
        else if(event->button() == Qt::RightButton)
        {
            if(m_bPolygonBegin)
            {
                m_bPolygonBegin = false;
                setMouseTracking(false);
                emit endPolyon(point.toPoint());
            }
        }
        else
        {

        }
    }
    else if(m_mode == ARROW)
    {
        QPointF point = this->mapToScene(event->x(), event->y());
        auto s = getSelect(point.toPoint());
        if(s != nullptr)
        {
            s->setSelected(true);
            emit selectChanged(s);
        }
    }
    else if(m_mode == SHOW)
    {
        m_mouse_x = event->x();
        m_mouse_y = event->y();
    }
}

void myView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(m_mode == DRAW)
    {
        if(bTrack)
        {
            bTrack = false;
            emit endTrack();
        }
    }
    else
    {
            m_p1 = event->pos();
    }

}


void myView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = this->mapToScene(event->x(), event->y());
    if(m_mode == DRAW)
    {
        if(bTrack)
        {
            m_p2 = point.toPoint();
            emit track(m_p1, m_p2);
        }
    }
    else if(m_mode == POLYGON)
    {
        if(m_bPolygonBegin)
        {
            emit trackPolyon(point.toPoint());
        }
    }
    else if(m_mode == ARROW)
    {
        emit lookAt(point.toPoint());
    }
    else if(m_mode == SHOW)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            QPointF old_point = this->mapToScene(m_mouse_x, m_mouse_y);
            float dx = point.x() - old_point.x();
            float dy = point.y() - old_point.y();
            view_x = view_x  - dx * 0.5;
            view_y = view_y - dy * 0.5;
            centerOn(view_x, view_y);
            m_mouse_x = event->x();
            m_mouse_y = event->y();
            emit viewCofChange(view_x, view_y,
                               view_sx, view_sy);
        }

    }
}

void myView::fitView(qreal x, qreal y, qreal w, qreal h)
{
    view_x = x + w * 0.5;
    view_y = y + h * 0.5;
    float m_w = width();
    float  m_h = height();
    float ratio_w = m_w / float(w+ 1);
    float ratio_h = m_h / float(h + 1);
    if(ratio_w > ratio_h)
    {
        view_sx = ratio_h;
        view_sy = ratio_h;

    }else
    {
        view_sx = ratio_w;
        view_sy = ratio_w;
    }
    QGraphicsView::fitInView(x, y, w, h, Qt::AspectRatioMode::KeepAspectRatio);
}

void myView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(m_mode == ARROW)
    {
        if(event->button() == Qt::LeftButton)
        {
            QPoint p1 = mapToScene(event->x(), event->y()).toPoint();
            emit dbClicked(p1);
        }
    }
    //QGraphicsView::mouseDoubleClickEvent(event);
}




QGraphicsItem* myView::getSelect(QPoint p)
{
    auto ilst = scene()->items();
    QList<QGraphicsItem*> candidates;
    QList<int> distance;
    for(auto item: ilst)
    {
        int t = item->type();
        item->setSelected(false);
        if(t != 7)
        {
           if(item->boundingRect().contains(p))
           {
               candidates.push_back(item);
               QPoint cen = item->boundingRect().center().toPoint();
               QPoint diff = cen - p;
               distance.push_back(diff.manhattanLength());
           }
        }
    }

    if(candidates.size() > 0)
    {
        int minDis = INT_MAX;
        int minID = 0;
        for(int i = 0; i < candidates.size(); i++)
        {
            if(minDis > distance[i])
            {
                minID = i;
                minDis = distance[i];
            }
        }
        return candidates[minID];
    }
    else
    {
        return nullptr;
    }
}

void pcie::getLookAt(QPoint p)
{
    for(auto item: m_pScene->items())
    {
        int t = item->type();
        item->setSelected(false);
        if(t == 7)
        {
           emit viewDetail(m_image, p);
           break;
        }
    }

}

void pcie::showGraphics()
{
    for(auto item: m_pScene->items())
    {
        int t = item->type();
        if(t != 7)
        {
            item->setVisible(true);
        }
    }
}


void pcie::hideGraphics()
{
    for(auto item: m_pScene->items())
    {
        int t = item->type();
        if(t != 7)
        {
            item->setVisible(false);
        }
    }
}


void myRectRegion::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    if(this->isSelected())
    {
        QPen pen = this->pen();
        pen.setStyle(Qt::DashDotDotLine);
        pen.setWidth(8);
        pen.setColor(Qt::green);
        painter->setPen(pen);
        if(m_bfill)
        {
            painter->setBrush(this->brush());
        }
        switch (m_type)
        {
            case 0:
                painter->drawRect(QRect(m_p1, m_p2).normalized());
                break;
            case 1:
                painter->drawEllipse(QRect(m_p1 - QPoint(10, 10), m_p1 + QPoint(10, 10)));
                painter->drawEllipse(QRect(m_p2 - QPoint(10, 10), m_p2 + QPoint(10, 10)));
                painter->drawLine(m_p1, m_p2);
                break;
            case 2:
                QPoint dist = m_p1 - m_p2;
                int r = dist.manhattanLength();
                painter->drawEllipse(m_p1, r, r);
                break;
        }

    }
    else
    {
        if(m_bfill)
        {
            painter->setBrush(this->brush());
        }
        QPen pen = this->pen();
        pen.setWidth(8);
        pen.setColor(Qt::red);
        painter->setPen(pen);
        switch (m_type)
        {
            case 0:
                painter->drawRect(QRect(m_p1, m_p2).normalized());
                break;
            case 1:
                painter->drawEllipse(QRect(m_p1 - QPoint(10, 10), m_p1 + QPoint(10, 10)));
                painter->drawEllipse(QRect(m_p2 - QPoint(10, 10), m_p2 + QPoint(10, 10)));
                painter->drawLine(m_p1, m_p2);
                break;
            case 2:
                QPoint dist = m_p1 - m_p2;
                int r = dist.manhattanLength();
                painter->drawEllipse(m_p1, r, r);
                break;
        }
    }
}

void pcie::viewCofChange(int x, int y, float sx, float sy)
{
    emit pcieViewChange(x, y, sx, sy);
}
