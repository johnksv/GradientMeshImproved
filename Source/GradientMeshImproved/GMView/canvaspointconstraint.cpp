#include "canvaspointconstraint.h"
#include <QDebug>
#include <QPainter>
#include <QVariant>
#include <QLineF>
#include <QtMath>
#include <QApplication>
#include "canvas.h"

using namespace GMView;

CanvasPointConstraint::CanvasPointConstraint(QGraphicsItem *controlPoint, CanvasItemLine *edge, QGraphicsItem *parent)
    : edge_(edge), QGraphicsItem(parent)
{
    setParentItem(controlPoint);

    CanvasItemPoint* point = dynamic_cast<CanvasItemPoint*> (controlPoint);
    if(point == nullptr)
    {
        CanvasPointDiscontinued* pointDis = dynamic_cast<CanvasPointDiscontinued*> (controlPoint);
        if(pointDis != nullptr)
        {
            controlPointDis_ = pointDis;
        }
        else
        {
            throw "Illegal parent type";
        }

    }
    else
    {
        controlPoint_ = point;
    }

    setZValue(2);
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges | ItemSendsScenePositionChanges);
}

QRectF CanvasPointConstraint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasPointConstraint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(!inactive_)
    {
        GMCanvas *canvas = static_cast<GMCanvas*>(scene());
        if(canvas->renderConstraintHandlers())
        {
            QColor color = QColor(125,125,125,125);
            painter->setBrush(color);
            painter->drawEllipse(boundingRect());
            //For debugging purposes TODO: remove
            if(controlPoint_ != nullptr)
            {
                painter->drawLine(mapFromScene(controlPoint_->pos()), QPoint(0,0));
                painter->drawText(QPointF(10,10),QString(QString::number(controlPoint_->vertexHandleIdx())));
            }else if(controlPointDis_ != nullptr)
            {
                QPointF startPosition = mapFromItem(controlPointDis_, controlPointDis_->pos()) - controlPointDis_->pos();
                painter->drawLine(startPosition, QPoint(0,0));
                painter->drawText(QPointF(10,10),QString(QString::number(controlPointDis_->vertexHandleIdx())));
            }
        }
    }
}

QPainterPath CanvasPointConstraint::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

QGraphicsItem *CanvasPointConstraint::controlPoint()
{
    return controlPoint_;
}

CanvasItemLine *CanvasPointConstraint::edge()
{
    return edge_;
}

void CanvasPointConstraint::setInactive(bool value)
{
    inactive_ = value;
}

QVariant CanvasPointConstraint::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange || change == ItemScenePositionHasChanged)
    {
        edge_->updateSubdivisonCurve();
    }
    return value;
}
