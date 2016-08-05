#include "canvaspointdiscontinued.h"
#include "GUILogic/meshhandler.h"

using namespace GMView;

CanvasPointDiscontinued::CanvasPointDiscontinued(bool sameIdxAsParent, QGraphicsItem *parent) : sameIdxAsParent_(sameIdxAsParent),QGraphicsItem(parent)
{
    setAcceptHoverEvents(true);

    CanvasItemPoint *parentPoint = dynamic_cast<CanvasItemPoint *>(parent);
    if(parentPoint != nullptr)
    {
        if(sameIdxAsParent_)
        {
            vertexHandleIdx_ = parentPoint->vertexHandleIdx();
        }
        else
        {
            GMCanvas *canvas = static_cast<GMCanvas*> (parentPoint->scene());
            QPointF position = mapToScene(pos());
            vertexHandleIdx_ = canvas->currentMeshHandler()->addVertex(position);
        }
    }
    else
    {
        throw "Parent should be a CanvasItemPoint";
    }
}

QRectF CanvasPointDiscontinued::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasPointDiscontinued::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(color_);
    painter->drawEllipse(boundingRect());
}

QPainterPath CanvasPointDiscontinued::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

QVariant CanvasPointDiscontinued::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    //TODO: Change of position
    return value;
}

QColor CanvasPointDiscontinued::color() const
{
    return color_;
}

void CanvasPointDiscontinued::setColor(QColor &color)
{
    this->color_ = color;
    GMCanvas* canvas = static_cast <GMCanvas*> (scene());
    canvas->currentMeshHandler()->setVertexColor(vertexHandleIdx_, color_);
    update(boundingRect());
}
