#include "canvaspointdiscontinued.h"
#include "GUILogic/meshhandler.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QAction>
#include <QColorDialog>
#include "canvas.h"

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
            //Minor offset for not crashing normals with OpenMesh
            QPointF position = mapToScene(pos())-QPointF(0.01,0.01);
            vertexHandleIdx_ = canvas->currentMeshHandler()->addVertex(position);
        }
    }
    else
    {
        throw "Parent should be a CanvasItemPoint";
    }

    //Position is updated by parents ItemChange-function
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

void CanvasPointDiscontinued::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *chooseColorAction = menu.addAction("Choose color");
    menu.addSeparator();

    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColor chosenColor = QColorDialog::getColor();
        if(chosenColor.isValid())
        {
            setColor(chosenColor);
        }

    }
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

int CanvasPointDiscontinued::vertexHandleIdx() const
{
    return vertexHandleIdx_;
}

void CanvasPointDiscontinued::setVertexHandleIdx(int idx)
{
    vertexHandleIdx_ = idx;
}

bool CanvasPointDiscontinued::sameIdxAsParent() const
{
    return sameIdxAsParent_;
}
