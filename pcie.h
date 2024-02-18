#ifndef PCIE_H
#define PCIE_H
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPushButton>
#include <QDebug>

enum PCIEMod
{
    SHOW,
    DRAW,
    POLYGON,
    ARROW
};

class  myView: public QGraphicsView
{
Q_OBJECT
public:
    myView(QWidget* parent):QGraphicsView(parent){}
    void setMode(PCIEMod mode);
    QGraphicsItem* getSelect(QPoint p);
    void fitView(qreal x, qreal y, qreal w, qreal h);
    void centerOn(qreal x, qreal y)
    {
        view_x = x;
        view_y = y;
        QGraphicsView::centerOn(x, y);
    }
signals:
    void beginTrack();
    void track(QPoint, QPoint);
    void endTrack();
    void beginPolyon(QPoint);
    void updatePolygon(QPoint);
    void trackPolyon(QPoint);
    void endPolyon(QPoint);
    void dbClicked(QPoint);
    void selectChanged(QGraphicsItem*);
    void lookAt(QPoint);
    void viewCofChange(int x, int y, float sx, float sy);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
public:
    PCIEMod m_mode = SHOW;
    int  view_x = 0;
    int  view_y = 0;
    float view_sx = 1.0;
    float view_sy = 1.0;
private:
    bool bTrack = false;
    bool m_bPolygonBegin = false;
    QPoint m_p1;
    QPoint m_p2;
    float m_mouse_x;
    float m_mouse_y;
};

class myRectRegion:public QGraphicsRectItem
{
public:
    myRectRegion(const QRectF &rect,  int type = 0, bool fill = false, QGraphicsItem *parent = nullptr):
        QGraphicsRectItem(rect, parent), m_bfill(fill), m_type(type)
    {
       setFlag(QGraphicsItem::ItemIsSelectable, true);
       m_p1 = rect.topLeft().toPoint();
       m_p2 = rect.bottomRight().toPoint();
    };
    QRectF boundingRect() const override
    {
        switch (m_type)
        {
            //矩形模式
            case 0:
                return QRectF(m_p1, m_p2).normalized().marginsAdded(QMarginsF(10, 10, 10, 10));
            case 1:
                return QRectF(m_p1, m_p2).normalized().marginsAdded(QMarginsF(10, 10, 10, 10));
            case 2:
                QPoint dist = m_p1 - m_p2;
                int r = dist.manhattanLength();
                return QRectF(m_p1 - QPoint(r, r), m_p1 + QPoint(r, r)).normalized().marginsAdded(QMarginsF(10, 10, 10, 10));
        }
        return QRectF(m_p1, m_p2).normalized();
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void setPoints(QPoint p1, QPoint p2)
    {
        m_p1 = p1;
        m_p2 = p2;
    }

    void setName(std::string name)
    {
        this->setData(Qt::UserRole + 1, QString(name.c_str()));
    }

private:
    bool m_bfill;
public:
    QPoint m_p1;
    QPoint m_p2;
    int m_type;
};

class myPixmapGraphics:public QGraphicsPixmapItem
{
public:
    enum { Type = UserType + 1 };
    myPixmapGraphics(QPixmap pixmap,QGraphicsItem *parent = nullptr):
        QGraphicsPixmapItem(pixmap, parent)
    {

    }
    void setName(std::string name)
    {
        this->setData(Qt::UserRole + 1, QString(name.c_str()));
    }
    int type() const override
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }
};


class  pcie:public QWidget
{
Q_OBJECT
public:
    pcie(QWidget* parent);
    bool setImage(QImage& im);
    void setMode(PCIEMod mode);
    PCIEMod getMode(){return  m_pView->m_mode;};
    void setRect(QRect, int r, int g, int b, bool fill);
    void setPolygon(std::vector<float>& x, std::vector<float>& y,
                    int r, int g, int b, bool fill);
    void setCircle(float x, float y, float ra,
                   int r, int g, int b, bool fill);
    void setText(QString, int, int, int);
    void setLine(float x1, float y1, float x2,
                 float y2, int r, int g, int b, bool fill);
    void setRuler(float x1, float y1, float x2,
                  float y2, int r, int g, int b, bool fill);
    void setVAERegion(QPoint p1, QPoint p2, int r, int g, int b,
                      int type, std::string name);
    void setImageMask(QImage&, std::string name);
    void setTrackerType(int type);
    void cleargeo();
    void clearImage();
    void removeOneGeo(QGraphicsItem* item);
    void resizeEvent(QResizeEvent *event) override
    {
        this->on_Fit();
        QWidget::resizeEvent(event);
    }
    void setViewCOff(int x, int y, float sx, float sy);
signals:
    void rectDraw(QPoint, QPoint);
    void rectEdit(std::string);
    void maskPolyon(QList<QPoint>);
    void viewDetail(QImage&, QPoint);
    void pcieViewChange(int x, int y, float sx, float sy);
public slots:
    void viewCofChange(int x, int y, float sx, float sy);
    void on_Fit();
    void on_BeginTrack();
    void on_UpdateTracker(QPoint p1, QPoint p2);
    void on_endTrack();
    //删除路径
    void on_BeginPolyon(QPoint);
    void on_UpdatePolyon(QPoint);
    void on_EndPolyon(QPoint);
    void trackPolyon(QPoint);
    // 获取选中图元
    QGraphicsItem* getChoose(std::string& name);
    // 双击图元
    void on_DbClick(QPoint p);
    // 图元切换
    void selectedChange(QGraphicsItem*);

    // 获取当前鼠标停留点
    void getLookAt(QPoint);

    // 隐藏所有图元
    void hideGraphics();

    // 显示所有图元
    void showGraphics();
public:
        myView* m_pView;
private:

    QGraphicsScene* m_pScene;
    myRectRegion* m_RtTracker;
    QGraphicsPolygonItem* m_PolygonTracker;
    QGraphicsItem* m_curChooseGraphic;
    QPushButton* m_BtnZoomFit;
    int m_ntype;
    QImage m_image;
};

#endif // PCIE_H
