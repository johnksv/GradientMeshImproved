#include "canvasitemface.h"
#include <QPainter>

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

    for(int i =0; i <  edgesInFace_.size(); i++){
        QPointF pos =  edgesInFace_.at(i)->startPoint()->pos();
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
    for (int i = 0; i < edgesInFace_.size(); ++i) {
        vector<QPointF> points = edgesInFace_.at(i)->subdivededCurve();
        for (int j = 0; j < points.size(); ++j) {
            poly.push_back(points.at(j));
        }
    }

    path.addPolygon(poly);
    return path;
}

void CanvasItemFace::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
     painter->setBrush(QColor(225,225,225,100));
     painter->setPen(Qt::NoPen);
     painter->drawPath(shape());
}

void CanvasItemFace::addCanvasEdge(CanvasItemLine *edge)
{
    edgesInFace_.push_back(edge);
}
