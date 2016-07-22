#include "canvasitemline.h"
#include <QDebug>

CanvasItemLine::CanvasItemLine(CanvasItemPoint *startPoint, CanvasItemPoint *endPoint, QGraphicsItem *parent) : QGraphicsLineItem(parent), startPoint_(startPoint), endPoint_(endPoint)
{
    setLine(QLineF(startPoint->pos(), endPoint->pos()));
    setZValue(1);
    //setFlags(ItemIsSelectable);
}

void CanvasItemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setLine(QLineF(startPoint_->pos(), endPoint_->pos()));
    QGraphicsLineItem::paint(painter,option,widget);
}

bool CanvasItemLine::operator ==(const CanvasItemLine &lineA)
{
    if((*this).startPoint_ == lineA.startPoint_)
    {
        if((*this).endPoint_ == lineA.endPoint_)
        {
            return true;
        }
    }
    return false;
}

GUILogic::OpnMeshEdgeHandle CanvasItemLine::edgeHandle()
{
    return edge_;
}

void CanvasItemLine::setEdgeHandle(GUILogic::OpnMeshEdgeHandle edgeHandle)
{
    edge_ = edgeHandle;
}
