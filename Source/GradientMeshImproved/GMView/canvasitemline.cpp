#include "canvasitemline.h"
#include <QDebug>

CanvasItemLine::CanvasItemLine(CanvasItemPoint *startPoint, CanvasItemPoint *endPoint, QGraphicsItem *parent) : QGraphicsLineItem(parent), startPoint(startPoint), endPoint(endPoint)
{
    setLine(QLineF(startPoint->pos(), endPoint->pos()));
    setZValue(1);
    //setFlags(ItemIsSelectable);
}

void CanvasItemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setLine(QLineF(startPoint->pos(), endPoint->pos()));
    QGraphicsLineItem::paint(painter,option,widget);
}
