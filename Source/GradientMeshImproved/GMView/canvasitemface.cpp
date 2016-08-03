#include "canvasitemface.h"

#include <QPainter>
#include <QDebug>
#include "canvaspoint.h"

using namespace GMView;

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
        QPointF pos = item->pos();
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

QPainterPath CanvasItemFace::shape() const
{
    QPainterPath path;
    QPolygonF poly;
    for(CanvasItemPoint *item : pointsInFace_)
    {
        poly.push_back(item->pos());
    }
    path.addPolygon(poly);
    return path;
}

void CanvasItemFace::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    //TODO: Remove Face from drawing.
    painter->setBrush(QColor(225,225,225,100));

    painter->drawPath(shape());
}

void CanvasItemFace::addCanvasPoint(CanvasItemPoint *point)
{
    pointsInFace_.push_back(point);
}
