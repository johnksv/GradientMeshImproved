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

using namespace GMView;

CanvasItemPoint::CanvasItemPoint(QColor color, QGraphicsItem *parent):
    QGraphicsItem(parent), color_(color)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if(isSelected()){
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

    if(hovered_){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1);
    }

    if(!discontinuous_) painter->drawEllipse(boundingRect());

    //For debugging purposes TODO: remove
    painter->drawText(QPointF(10,10),QString(QString::number(vertexHandleIdx())));
    QString stringPos = QString::number(pos().x()).append(", ").append(QString::number(pos().y()));
    painter->drawText(QPointF(10,20),stringPos);
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

bool CanvasItemPoint::discontinuous() const
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

            pointA->setPos(0,-10);
            pointB->setPos(0,10);
            double factor;

            for(QGraphicsItem *item: childItems())
            {
                CanvasPointConstraint *constraint = dynamic_cast<CanvasPointConstraint*> (item);
                if(constraint != nullptr)
                {
                    constraint->setInactive(true);
                    CanvasItemLine *consEdge = constraint->edge();
                    factor = (this == consEdge->startPoint()) ? 0.2 : -0.2;

                    CanvasPointConstraint *pointAConstraint = new CanvasPointConstraint(pointA, consEdge);
                    pointAConstraint->setPos(QPointF(consEdge->line().dx()*factor, consEdge->line().dy()*factor));

                    CanvasPointConstraint *pointBConstraint = new CanvasPointConstraint(pointB, consEdge);
                    pointBConstraint->setPos(QPointF(consEdge->line().dx()*factor, consEdge->line().dy()*factor));
                }

            }

            canvasPointDiscontinuedChildren_.push_back(pointA);
            canvasPointDiscontinuedChildren_.push_back(pointB);
        }
    }
    else
    {
        for(QGraphicsItem* item : canvasPointDiscontinuedChildren_)
        {
            delete item;
        }
        canvasPointDiscontinuedChildren_.clear();

        for(QGraphicsItem *item: childItems())
        {
            CanvasPointConstraint *constraint = dynamic_cast<CanvasPointConstraint*> (item);
            if(constraint != nullptr)
            {
                constraint->setInactive(false);
            }

        }


        //TODO: Delete the discontinued points (Internaly maybe with collapsing edges)
    }

    update();
}

QGraphicsItem *CanvasItemPoint::controlPoint(QGraphicsItem *_edge)
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
        GMCanvas* canvas = static_cast <GMCanvas*> (scene());
        if(discontinuous_)
        {
            for(QGraphicsItem * item : canvasPointDiscontinuedChildren_)
            {
                CanvasPointDiscontinued *disPoint = static_cast<CanvasPointDiscontinued*> (item);
                int vertIdx = disPoint->vertexHandleIdx();
                //Use same position as parent
                canvas->currentMeshHandler()->setVertexPoint(vertIdx, pos());
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
    hovered_ = true;
    update();
}

void CanvasItemPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    hovered_ = false;
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
