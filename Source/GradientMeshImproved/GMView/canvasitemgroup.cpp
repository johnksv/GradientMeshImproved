#include "canvasitemgroup.h"
#include <QPainter>
#include <QDebug>

using namespace GMView;

CanvasItemGroup::CanvasItemGroup(QString layername, QGraphicsItem *parent) :
    QGraphicsItem(parent), QStandardItem(layername)
{
    setHandlesChildEvents(false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);
}

QRectF CanvasItemGroup::boundingRect() const
{
    return childrenBoundingRect();
}

QPainterPath CanvasItemGroup::shape() const
{
    QPainterPath result;

    for (int i = 0; i < faces.size(); i++)
    {
        result.addPath(faces.at(i)->shape());
    }
    return result;
}

void CanvasItemGroup::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void CanvasItemGroup::addToGroup(QGraphicsItem *item)
{
    //TODO: Auto place item in correct vector.
    item->setParentItem(this);
}

void CanvasItemGroup::removeFromGroup(QGraphicsItem *item)
{
    //Todo: auto remove item from correct vector;
    item->setParentItem(0);
}
