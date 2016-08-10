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
        else
        {
            CanvasItemFace *face = dynamic_cast<CanvasItemFace*> (item);
            if(face != nullptr)
            {
                faces.push_back(face);
            }
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
        else
        {
            CanvasItemFace *face = dynamic_cast<CanvasItemFace*> (item);
            if(face != nullptr)
            {
                for(int i = 0 ; i < faces.size(); i++)
                {
                    if(faces.at(i) == face)
                    {
                        faces.erase(faces.begin()+i);
                        break;
                    }
                }
            }
        }
    }

    item->setParentItem(0);
}
