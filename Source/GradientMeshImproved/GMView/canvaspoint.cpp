#include "canvaspoint.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QColorDialog>
#include <QInputDialog>
#include "canvas.h"
#include <QPen>
#include "canvaspointdiscontinued.h"
#include <QStyleOptionGraphicsItem>

using namespace GMView;

CanvasItemPoint::CanvasItemPoint(QColor color, QGraphicsItem *parent):
    QGraphicsItem(parent), color_(color)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
    setZValue(2);

}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    if(!discontinuous_)
    {
        painter->setPen(Qt::NoPen);


        const qreal detailLevel = option->levelOfDetailFromTransform(painter->worldTransform());
        radius_ = 5 / detailLevel;
        if(radius_ < 0.5) radius_ = 0.5;

        if(option->state & QStyle::State_Selected){
            //TODO: Fix selected color
            QColor temp = QColor(color_);
            temp.setAlpha(150);
            painter->setBrush(temp);
        }
        else
        {
            if(highlighted_)
            {
                QPen pen(Qt::red);
                pen.setWidth(2);
                painter->setPen(pen);
            }
            painter->setBrush(color_);
        }

        if(option->state & QStyle::State_MouseOver){
            painter->setOpacity(0.5);
            //For debugging purposes TODO: remove
            painter->drawText(QPointF(10,10),QString(QString::number(vertexHandleIdx())));
            QString stringPos = QString::number(pos().x()).append(", ").append(QString::number(pos().y()));
            painter->drawText(QPointF(10,20),stringPos);
        }

        painter->drawEllipse(boundingRect());
    }
}

QPainterPath CanvasItemPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}


QColor CanvasItemPoint::color() const
{
    return color_;
}

void CanvasItemPoint::setColor(QColor color)
{
    this->color_ = color;
    GMCanvas* canvas = static_cast <GMCanvas*> (scene());
    canvas->currentMeshHandler()->setVertexColor(vertexHandleIdx_, color_);
    update(boundingRect());
}

int CanvasItemPoint::vertexHandleIdx() const
{
	return vertexHandleIdx_;
}

void CanvasItemPoint::setVertexHandleIdx(int idx)
{
    vertexHandleIdx_ = idx;
}

double CanvasItemPoint::radius() const
{
    return radius_;
}

void CanvasItemPoint::setRadius(int _radius)
{
    if(_radius<=0){
        radius_ = 1;
    }else{
        radius_ = _radius;
    }
}

bool CanvasItemPoint::isHighlighted() const
{
    return highlighted_;
}

void CanvasItemPoint::setHighlighted(bool highlighted)
{
    highlighted_ = highlighted;
}

bool CanvasItemPoint::isDiscontinuous() const
{
    return discontinuous_;
}

void CanvasItemPoint::setDiscontinuous(bool value, QGraphicsItem *edgeItem)
{
    discontinuous_ = value;
    CanvasItemLine *edge = dynamic_cast<CanvasItemLine*>(edgeItem);
    if(discontinuous_)
    {
        if(edge != nullptr)
        {
            CanvasPointDiscontinued *pointA = new CanvasPointDiscontinued(true, this);
            CanvasPointDiscontinued *pointB = new CanvasPointDiscontinued(false, this);
            //TODO: set position of A and B on the normal of the incoming edge for visual look.
            //TODO: Double constraints only for "spliting"-edge (edge on the middle)
            //This does not effect position in underlaying mesh.
            pointA->setPos(0,10);
            pointB->setPos(0,-10);

            discontinuedChildren_.push_back(pointA);
            discontinuedChildren_.push_back(pointB);
        }
    }
    else
    {
        for(QGraphicsItem* item : discontinuedChildren_)
        {
            delete item;
        }
        discontinuedChildren_.clear();
    }

    update();
}

vector<QGraphicsItem *> CanvasItemPoint::discontinuedChildren()
{
    return discontinuedChildren_;
}

QGraphicsItem *CanvasItemPoint::constraintPoint(QGraphicsItem *_edge)
{
    CanvasItemLine *edge = dynamic_cast<CanvasItemLine*> (_edge);
    if(edge != nullptr)
    {
        for(QGraphicsItem *item : childItems())
        {
            CanvasPointConstraint *constraint = dynamic_cast<CanvasPointConstraint*> (item);
            if(constraint != nullptr)
            {
                if(constraint->edge() == edge)
                {
                    return static_cast<QGraphicsItem*> (constraint);
                }
            }
        }

        return nullptr;
    }
    throw "Input must be CanvasItemLine";
}

QVariant CanvasItemPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged)
    {
        //Update position directly in meshhandler
        GMCanvas* canvas = static_cast <GMCanvas*> (scene());
        if(discontinuous_)
        {
            for(QGraphicsItem * item : discontinuedChildren_)
            {
                CanvasPointDiscontinued *disPoint = static_cast<CanvasPointDiscontinued*> (item);
                int vertIdx = disPoint->vertexHandleIdx();
                //Use same position as parent (+ minor offset, for opnmesh not to crash)
                canvas->currentMeshHandler()->setVertexPoint(vertIdx, pos()-QPointF(0.01,0.01));
            }
        }
        else
        {
            canvas->currentMeshHandler()->setVertexPoint(vertexHandleIdx_, pos());
        }


    }
    return QGraphicsItem::itemChange(change, value);
}

void CanvasItemPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
}

void CanvasItemPoint::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    update();
}

void CanvasItemPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    update();
}

void CanvasItemPoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *chooseColorAction = menu.addAction("Choose color");
    menu.addSeparator();

    QAction *deletePointAction = menu.addAction("Delete");
    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColor chosenColor = QColorDialog::getColor();
        if(chosenColor.isValid())
        {
            setColor(chosenColor);
        }

    }
    else if(selectedAction == deletePointAction)
    {
        //TODO Implement delete point
    }
}
