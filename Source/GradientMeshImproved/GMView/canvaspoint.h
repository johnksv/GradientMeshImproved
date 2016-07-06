#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>
#include <QRectF>
#include "GUILogic/meshhandler.h"

class CanvasItemPoint :public QGraphicsItem
{
public:

    CanvasItemPoint(QPointF pos = QPointF(0,0), QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    void setColor(QColor color);
    //TODO: read on  item and sceen coordinates;
    QPointF position();

    void setRadius(int);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QPointF  posit, oldMousePos;
    QColor color;
    int radius = 5;
    bool hovered = false;
};

#endif // GMCANVASITEM_H
