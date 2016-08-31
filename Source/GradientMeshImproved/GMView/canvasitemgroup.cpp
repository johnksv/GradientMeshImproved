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
        points.push_back(point);
    }
    else
    {
        CanvasItemLine *line = dynamic_cast<CanvasItemLine*> (item);
        if(line != nullptr)
        {
            lines.push_back(line);
        }
    }

    item->setParentItem(this);
}

void CanvasItemGroup::removeFromGroup(QGraphicsItem *item)
{
    CanvasItemPoint *point = dynamic_cast<CanvasItemPoint*> (item);
    if(point != nullptr)
    {
        for(int i = 0 ; i < points.size(); i++)
        {
            if(points.at(i) == point)
            {
                points.erase(points.begin()+i);
                break;
            }
        }
    }
    else
    {
        CanvasItemLine *line = dynamic_cast<CanvasItemLine*> (item);
        if(line != nullptr)
        {
            for(int i = 0 ; i < lines.size(); i++)
            {
                if(lines.at(i) == line)
                {
                    lines.erase(lines.begin()+i);
                    break;
                }
            }
        }
    }

    item->setParentItem(0);
}

void CanvasItemGroup::resetPointsHighlighted()
{
    for (int i = 0; i < points.size(); ++i) {
        points.at(i)->setHighlighted(false);
    }
}
