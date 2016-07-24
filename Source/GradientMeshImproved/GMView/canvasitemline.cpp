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

bool CanvasItemLine::operator ==(const CanvasItemLine &lineA)
{
    if((*this).startPoint == lineA.startPoint)
    {
        if((*this).endPoint == lineA.endPoint)
        {
            return true;
        }
    }
    return false;
}

int CanvasItemLine::edgeHandleIdx()
{
    return edgeHandleIdx_;
}

void CanvasItemLine::setEdgeHandleIdx(int idx)
{
    edgeHandleIdx_ = idx;
}
