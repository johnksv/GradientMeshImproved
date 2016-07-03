#include "gmcanvasitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

GMCanvasItem::GMCanvasItem(QGraphicsItem *parent):
    QGraphicsItem(parent)
{

}

QRectF GMCanvasItem::boundingRect() const
{
    return QRectF(0,0,100,100);
}

void GMCanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void GMCanvasItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() <<"hoverEnter";
}

void GMCanvasItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() <<"hoverMove";
}
