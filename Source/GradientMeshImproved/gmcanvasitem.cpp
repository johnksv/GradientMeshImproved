#include "gmcanvasitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>



GMCanvasItem::GMCanvasItem(QGraphicsItem *parent, QPointF pos):
    QGraphicsItem(parent), position(pos)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}

QRectF GMCanvasItem::boundingRect() const
{
    return QRectF(position.x()-radius, position.y()-radius,radius*2,radius*2);
}

void GMCanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(color);
    if(hovered){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1);
    }
    painter->drawEllipse(position,radius,radius);
}

QPainterPath GMCanvasItem::shape() const
{
    QPainterPath path;
    path.addEllipse(position,radius,radius);
    return path;
}

void GMCanvasItem::setRadius(int _radius)
{
    if(_radius<=0){
        radius = 1;
    }else{
        radius = _radius;
    }
}

void GMCanvasItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

}

void GMCanvasItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = true;
    update();
}

void GMCanvasItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{

}

void GMCanvasItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered = false;
    update();
}

void GMCanvasItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
        QAction *choosColorAction = menu.addAction("Choose color");
        QAction *selectedAction = menu.exec(event->screenPos());

}
