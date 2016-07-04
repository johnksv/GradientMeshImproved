#include "canvasPoint.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>



CanvasItemPoint::CanvasItemPoint(QGraphicsItem *parent, QPointF pos):
    QGraphicsItem(parent), position(pos)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(position.x()-radius, position.y()-radius,radius*2,radius*2);
}

void CanvasItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(color);
    if(hovered){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1);
    }
    painter->drawEllipse(position,radius,radius);
}

QPainterPath CanvasItemPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(position,radius,radius);
    return path;
}

void CanvasItemPoint::setRadius(int _radius)
{
    if(_radius<=0){
        radius = 1;
    }else{
        radius = _radius;
    }
}

void CanvasItemPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

}

void CanvasItemPoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = true;
    update();
}

void CanvasItemPoint::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{

}

void CanvasItemPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = false;
    update();
}

void CanvasItemPoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
        QAction *choosColorAction = menu.addAction("Choose color");
        QAction *selectedAction = menu.exec(event->screenPos());

}
