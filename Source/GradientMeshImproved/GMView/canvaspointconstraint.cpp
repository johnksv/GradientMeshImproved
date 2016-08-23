#include "canvaspointconstraint.h"
#include <QDebug>
#include <QPainter>
#include <QVariant>
#include <QLineF>
#include <QtMath>
#include <QApplication>
#include "canvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

using namespace GMView;

CanvasPointConstraint::CanvasPointConstraint(CanvasItemPoint *controlPoint, CanvasItemLine *edge, QGraphicsItem *parent)
    : controlPoint_(controlPoint), edge_(edge), QGraphicsItem(parent)
{
    setParentItem(controlPoint);
    setZValue(2);
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges | ItemSendsScenePositionChanges);
}

QRectF CanvasPointConstraint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasPointConstraint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
        GMCanvas *canvas = static_cast<GMCanvas*>(scene());
        if(canvas->renderConstraintHandlers())
        {
            painter->setPen(Qt::NoPen);

            const qreal detailLevel = option->levelOfDetailFromTransform(painter->worldTransform());
            radius_ = 3 / detailLevel;
            if(radius_ < 0.3) radius_ = 0.3;

            QColor color = controlPoint()->color();
            painter->setBrush(color);
            painter->drawEllipse(boundingRect());

            QPen pen;
            QVector<qreal> dashes;
            QLineF line(mapFromScene(controlPoint_->pos()), QPoint(0,0));
            dashes << 0 << controlPoint()->radius() << line.length()-radius_ << 0;

            pen.setDashPattern(dashes);
            pen.setWidthF(1/detailLevel);
            painter->setPen(pen);
            painter->drawLine(line);
        }
}

QPainterPath CanvasPointConstraint::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

CanvasItemPoint *CanvasPointConstraint::controlPoint()
{
    return controlPoint_;
}

CanvasItemLine *CanvasPointConstraint::edge()
{
    return edge_;
}

QVariant CanvasPointConstraint::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange || change == ItemScenePositionHasChanged)
    {
        edge_->updateSubdivisonCurve();

        //Update position in underlying mesh
        GMCanvas *canvas = static_cast<GMCanvas*>(scene());
        if(canvas == nullptr)
        {
            qDebug() << "Scene" << parentItem()->scene();
            return value;
        }

        int timesToLoop = 1;
        int controlPointIdx;

        //If discontinued: go thorugh each childPoint and assign this gradientConstriant vector.
        //TODO: Test (should probably implement support for discontinuity first.
        bool discontinued = controlPoint_->isDiscontinuous();
        if(discontinued) timesToLoop = controlPoint_->discontinuedChildren().size();

        for(int i =0; i < timesToLoop; i++)
        {
            if(discontinued)
            {
                controlPointIdx = static_cast<CanvasPointDiscontinued*>(controlPoint_->discontinuedChildren().at(i))->vertexHandleIdx();
            }
            else
            {
                controlPointIdx = controlPoint_->vertexHandleIdx();
            }

            int toVertexIdx;
            if(controlPoint_ == edge_->startPoint())
            {
                toVertexIdx = edge_->endPoint()->vertexHandleIdx();
            }
            else
            {
                toVertexIdx = edge_->startPoint()->vertexHandleIdx();
            }

            QVector2D constraint(pos());
            canvas->currentMeshHandler()->setConstraints(controlPointIdx, toVertexIdx, constraint);
        }



    }
    return value;
}

void CanvasPointConstraint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    GMCanvas *canvas = static_cast<GMCanvas*>(scene());
    if(!canvas->renderConstraintHandlers())
    {
        event->ignore();
    }else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}
