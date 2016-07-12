#include "canvasitemface.h"

#include <QPainter>
#include <QDebug>
#include "canvaspoint.h"

CanvasItemFace::CanvasItemFace(QGraphicsItem *parent) : QGraphicsItem(parent)
{

}

QRectF CanvasItemFace::boundingRect() const
{
    qreal   top = std::numeric_limits<int>::max(),
            left = std::numeric_limits<int>::max(),
            right = std::numeric_limits<int>::min(),
            bottom = std::numeric_limits<int>::min();

    for(CanvasItemPoint *item : pointsInFace_)
    {
        QPointF pos = item->position();
        if(pos.x() < left)
        {
            left = pos.x();
        }
        if(pos.x() > right)
        {
            right = pos.x();
        }
        if(pos.y() < top)
        {
            top = pos.y();
        }
        if(pos.y() > bottom)
        {
            bottom = pos.y();
        }
    }
    return QRectF(left,top,right-left,bottom-top);
}

void CanvasItemFace::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(Qt::gray);
    QPolygonF temp;
    for(CanvasItemPoint *item : pointsInFace_)
    {
        temp.push_back(item->position());
    }
    painter->drawPolygon(temp);
}

void CanvasItemFace::addCanvasPoint(CanvasItemPoint *point)
{
    pointsInFace_.push_back(point);
}
