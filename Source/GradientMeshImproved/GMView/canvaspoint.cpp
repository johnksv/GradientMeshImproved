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
#include <QString>

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
//        QString string("x: ");
//        string.append(QString::number(x())).append("y: ")
//                .append(QString::number(y()));
//        QString string1("x: ");
//        string1.append(QString::number(vertexHandleIdx_));
//        painter->drawText(10,10, string1);
        painter->setPen(Qt::NoPen);

        const qreal detailLevel = option->levelOfDetailFromTransform(painter->worldTransform());
        radius_ = 5.8 / detailLevel;
        if(radius_ < 0.5) radius_ = 0.5;

        if(option->state & QStyle::State_Selected){

            QColor temp = QColor(color_);
            painter->setBrush(temp);
            QPen pen(Qt::red);
            //TODO: Set better width
            pen.setWidth(2);
            painter->setPen(pen);
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

void CanvasItemPoint::setColor(QColor color, bool callAutoRender)
{
    this->color_ = color;
    GMCanvas* canvas = static_cast <GMCanvas*> (scene());
    canvas->currentMeshHandler()->setVertexColor(vertexHandleIdx_, color_);
    if(callAutoRender)
    {
        canvas->autoRenderOnMeshChanged();
    }
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
    if(change == ItemPositionChange)
    {
        if(scene() == nullptr) return QGraphicsItem::itemChange(change, value);

        QPointF position = value.toPointF();
        QRectF scenerect = scene()->sceneRect();

        if(position.x() > scenerect.x()+scenerect.width() || position.x() < scenerect.x())
        {
            if(position.y() > scenerect.y() + scenerect.height() || position.y() < scenerect.y())
            {
                return QGraphicsItem::itemChange(change, pos());
            }else
            {
                return QGraphicsItem::itemChange(change, QPointF(x(),position.y()));
            }
        }else if(position.y() > scenerect.y() + scenerect.height() || position.y() < scenerect.y())
        {
            return QGraphicsItem::itemChange(change, QPointF(position.x(),y()));
        }
    }
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

void CanvasItemPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    if(event->button() == Qt::LeftButton) static_cast<GMCanvas*>(scene())->autoRenderOnMeshChanged();
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
    GMCanvas* canvas = static_cast <GMCanvas*> (scene());

    QMenu menu;
    QAction *chooseColorAction = menu.addAction("Choose color");
    menu.addSeparator();

    QAction *deletePointAction = menu.addAction("Delete");
    if(canvas->currentMeshHandler()->vertexValence(vertexHandleIdx_) < 1) deletePointAction->setEnabled(false);

    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColor chosenColor = QColorDialog::getColor();
        if(chosenColor.isValid())
        {
            QList<QGraphicsItem *> items = canvas->selectedItems();
            for(int i = 0; i < items.size(); ++i)
            {
                CanvasItemPoint * item = dynamic_cast<CanvasItemPoint *> (items.at(i));
                if(item != nullptr)
                {
                    item->setColor(chosenColor);
                }
            }
            //To guarantee that the color on this object is set.
            setColor(chosenColor);
        }

    }
    else if(selectedAction == deletePointAction)
    {

        canvas->currentMeshHandler()->deleteVertex(vertexHandleIdx_);
        canvas->currentMeshHandler()->garbageCollectOpenMesh();
        canvas->clearAllCurrLayer(false);
        canvas->constructGuiFromMeshHandler();
        canvas->autoRenderOnMeshChanged();
    }
}
