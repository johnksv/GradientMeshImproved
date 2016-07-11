#include "canvasitemline.h"
#include <QDebug>

CanvasItemLine::CanvasItemLine(CanvasItemPoint *startPoint, CanvasItemPoint *endPoint, QGraphicsItem *parent) : QGraphicsLineItem(parent), startPoint(startPoint), endPoint(endPoint)
{
    setLine(QLineF(startPoint->position(), endPoint->position()));
    setZValue(1);

    // setFlags(ItemIsSelectable);
}
