#include "canvasitemgroup.h"

CanvasItemGroup::CanvasItemGroup(QString layername, QGraphicsItem *parent) :
    QGraphicsItemGroup(parent), QStandardItem(layername)
{
    setHandlesChildEvents(false);
}
