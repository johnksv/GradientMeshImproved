#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>
#include <QRectF>
#include "GUILogic/meshhandler.h"

class CanvasItemPoint : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    CanvasItemPoint(QPointF pos = QPointF(0,0),QColor color = QColor(Qt::black), QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    //TODO: read on  item and sceen coordinates;
    QPointF position();
    QColor color();
    double weight();

    void setRadius(int);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QPointF  position_, oldMousePos_;
    QColor color_;
    double weight_;
    int radius_ = 5;
    bool hovered_ = false;

public slots:
    void setColor(QColor color);
    void setWeight(double);
};

#endif // GMCANVASITEM_H
