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

    points.setZValue(2);
	points.setParentItem(this);
    points.setHandlesChildEvents(false);
    points.setFlag(ItemIsSelectable, false);
    points.setFlag(ItemIsMovable, false);

	lines.setParentItem(this);
	lines.setHandlesChildEvents(false);
    lines.setFlag(ItemIsSelectable, false);
	lines.setFlag(ItemIsMovable, false);
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
        points.addToGroup(point);
		qDebug() << points.childItems();
    }
    else
    {
        CanvasItemLine *line = dynamic_cast<CanvasItemLine*> (item);
        if(line != nullptr)
        {
            lines.addToGroup(line);
        }
    }
}


void CanvasItemGroup::resetPointsHighlighted()
{
    foreach (QGraphicsItem* point, points.childItems()) {
        static_cast<CanvasItemPoint*> (point)->setHighlighted(false);
    }
}
