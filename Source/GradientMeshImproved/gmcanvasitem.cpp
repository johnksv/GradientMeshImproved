#include "gmcanvasitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>



GMCanvasItem::GMCanvasItem(QGraphicsItem *parent, QPointF pos):
    QGraphicsItem(parent), position(pos)
{
    qDebug() << position;
}

QRectF GMCanvasItem::boundingRect() const
{
return QRectF(0,0,10,10);
    return QRectF(position.x()-radius, position.y()-radius,
                  position.x()+radius,position.y()+radius);
}

void GMCanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawEllipse(position,radius,radius);
}

void GMCanvasItem::setRadius(int _radius)
{
    if(_radius<=0){
        radius = 1;
    }else{
        radius = _radius;
    }
}

void GMCanvasItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() <<"hoverEnter";
}

void GMCanvasItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() <<"hoverMove";
}
