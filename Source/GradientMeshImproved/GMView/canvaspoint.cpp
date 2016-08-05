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
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    parent->updateVertexFromPoint(*this, 1);
    update(boundingRect());
}

double CanvasItemPoint::weight() const
{
    return weight_;
}

void CanvasItemPoint::setWeight(double weight)
{
    this->weight_ = weight;
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    parent->updateVertexFromPoint(*this, 2);
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

void CanvasItemPoint::setDiscontinuous(bool value)
{
    discontinuous_ = value;
    if(discontinuous_)
    {
        CanvasPointDiscontinued *pointA = new CanvasPointDiscontinued(true, this);
        CanvasPointDiscontinued *pointB = new CanvasPointDiscontinued(false, this);
        //TODO set position of A and B on the normal of the incoming edge for visual look.
    }
    else
    {
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
        GMCanvas* parent = static_cast <GMCanvas*> (scene());
        parent->updateVertexFromPoint(*this, 0);
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
    QAction *setWeightAction = menu.addAction("Set weight");
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
    else if(selectedAction == setWeightAction)
    {
        QString label("Enter a value (0.00 - 1.00):");
        bool *ok = new bool;
        double newWeight = QInputDialog::getDouble(nullptr,"Vertex weight",label,weight_,0,1,2,ok, Qt::Tool);
        if(*ok == true)
        {
            setWeight(newWeight);
        }

        delete ok;

    }
    else if(selectedAction == deletePointAction)
    {
        //TODO Implement delete point
    }
}
