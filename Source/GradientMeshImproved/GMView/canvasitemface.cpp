#include "canvasitemface.h"
#include <QPainter>
#include <QDebug>
#include "GMView/utils.h"
#include "canvas.h"
#include <QGraphicsSceneContextMenuEvent>

using namespace GMView;

CanvasItemFace::CanvasItemFace(QGraphicsItem *parent) : QGraphicsItem(parent)
{   
}

CanvasItemFace::CanvasItemFace(QGraphicsItem *parent, int faceIdx): QGraphicsItem(parent), faceIdx_(faceIdx)
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

        if(reverseEdge_.at(i)){
            for (int j = points.size()-1; j > 0; --j) {
                poly.push_back(points.at(j));
            }
        }else{
            for (int j = 0; j < points.size(); ++j) {
                poly.push_back(points.at(j));
            }
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


void CanvasItemFace::addCanvasEdge(vector<CanvasItemLine *> &edges)
{
    if(edges.size() < 3){
        throw "Not enough edges to make face";
    }

    edgesInFace_.insert(edgesInFace_.begin(), edges.begin(), edges.end());

    for (int i = 0; i < edgesInFace_.size(); ++i) {
        reverseEdge_.push_back(0);
    }

    CanvasItemLine*  frontEdge = edgesInFace_.front();
    CanvasItemLine*  edge = edgesInFace_.back();

    if(frontEdge->startPoint() != edge->endPoint())
    {
       reverseEdge_.front() = true;
    }

    for (int i = 1; i < edgesInFace_.size(); ++i)
    {
        CanvasItemLine*  prevEdge = edgesInFace_.at(i-1);
        CanvasItemLine*  edge = edgesInFace_.at(i);
        if(prevEdge->endPoint() != edge->startPoint())
        {
            if(!reverseEdge_.at(i-1))
            {
                reverseEdge_.at(i) = true;
            }
            else
            {
                if(prevEdge->startPoint() != edge->startPoint())
                {
                    reverseEdge_.at(i) = true;
                }
            }
        }else if(i==1)
        {
            reverseEdge_.front() = false;
        }
    }

    //TODO: For debugging: remove
//    for (int i = 0; i < edgesInFace_.size(); ++i) {
//        qDebug() << "startVert" << edgesInFace_.at(i)->startPoint()->vertexHandleIdx() << "endVert" << edgesInFace_.at(i)->endPoint()->vertexHandleIdx() <<"reverse" << reverseEdge_.at(i);
    //    }
}

int CanvasItemFace::faceIdx() const
{
    return faceIdx_;
}

void CanvasItemFace::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        qDebug() << faceIdx_;
    GMCanvas* canvas = static_cast <GMCanvas*> (scene());

    QMenu menu;
    QAction *deletePointAction = menu.addAction("Delete");

    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == deletePointAction)
    {
        canvas->currentMeshHandler()->deleteFace(faceIdx_, false);
        canvas->currentMeshHandler()->garbageCollectOpenMesh();
        CanvasItemGroup* parent = static_cast<CanvasItemGroup*> (parentItem());
        parent->removeFromFaces(this);
        delete this;
    }
}
