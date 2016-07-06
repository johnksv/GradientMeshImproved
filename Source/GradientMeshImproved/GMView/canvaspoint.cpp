#include "canvaspoint.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QVariant>



CanvasItemPoint::CanvasItemPoint(QPointF pos, QGraphicsItem *parent):
    QGraphicsItem(parent), posit(pos)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable |ItemIsSelectable |ItemSendsScenePositionChanges );
}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(posit.x()-radius, posit.y()-radius,radius*2,radius*2);
}

void CanvasItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if(isSelected()){
        painter->setBrush(QColor(255,100,0));
    }else{
        painter->setBrush(color);
    }
    if(hovered){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1);
    }
    painter->drawEllipse(posit,radius,radius);
}

QPainterPath CanvasItemPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(posit,radius,radius);
    return path;
}

QVariant CanvasItemPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged){
        qDebug() << "CanvasPoint 48:  Item moved";
    }
    return QGraphicsItem::itemChange(change, value);
}


void CanvasItemPoint::setColor(QColor color)
{
    this->color = color;
}

QPointF CanvasItemPoint::position()
{
    return posit;
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

void CanvasItemPoint::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    hovered = true;
    update();
}

void CanvasItemPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    hovered = false;
    update();
}

void CanvasItemPoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *choosColorAction = menu.addAction("Choose color");
    QAction *setWeightAction = menu.addAction("Set weight");
    QAction *selectedAction = menu.exec(event->screenPos());
}
