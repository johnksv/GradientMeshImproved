#include "canvasitemgroup.h"
#include <QPainter>
#include <QDebug>

using namespace GMView;

CanvasItemContainer::CanvasItemContainer()
{
}
QRectF CanvasItemContainer::boundingRect() const
{
    return childrenBoundingRect();
}

QPainterPath CanvasItemContainer::shape() const
{
    return QPainterPath();
}

void CanvasItemContainer::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

CanvasItemGroup::CanvasItemGroup(QString layername, QGraphicsItem *parent) :
    QGraphicsItem(parent), QStandardItem(layername)
{
    setHandlesChildEvents(false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);

    points_.setZValue(2);
    points_.setParentItem(this);
    points_.setHandlesChildEvents(false);
    points_.setFlag(ItemIsSelectable, false);
    points_.setFlag(ItemIsMovable, false);

    lines_.setParentItem(this);
    lines_.setHandlesChildEvents(false);
    lines_.setFlag(ItemIsSelectable, false);
    lines_.setFlag(ItemIsMovable, false);
}

QRectF CanvasItemGroup::boundingRect() const
{
    return childrenBoundingRect();
}

QPainterPath CanvasItemGroup::shape() const
{
    QPainterPath result;

    return result;
}

void CanvasItemGroup::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void CanvasItemGroup::addToGroup(QGraphicsItem *item)
{
    CanvasItemPoint *point = dynamic_cast<CanvasItemPoint*> (item);
    if(point != nullptr)
    {
        point->setParentItem(&points_);
    }
    else
    {
        CanvasItemLine *line = dynamic_cast<CanvasItemLine*> (item);
        if(line != nullptr)
        {
            line->setParentItem(&lines_);
        }
    }
}


void CanvasItemGroup::resetPointsHighlighted()
{
    foreach (QGraphicsItem* point, points_.childItems()) {
        static_cast<CanvasItemPoint*> (point)->setHighlighted(false);
    }
}

CanvasItemContainer &CanvasItemGroup::points()
{
    return points_;
}

CanvasItemContainer &CanvasItemGroup::lines()
{
    return lines_;
}
