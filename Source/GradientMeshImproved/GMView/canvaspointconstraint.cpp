#include "canvaspointconstraint.h"
#include <QDebug>
#include <QPainter>
#include <QVariant>
#include <QLineF>
#include <QtMath>
#include <QApplication>
#include "canvas.h"

using namespace GMView;

CanvasPointConstraint::CanvasPointConstraint(CanvasItemPoint *controlPoint, CanvasItemLine *edge, QGraphicsItem *parent)
    : controlPoint_(controlPoint), edge_(edge), QGraphicsItem(parent)
{
    setZValue(2);
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges | ItemSendsScenePositionChanges);
    setParentItem(controlPoint_);
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
        QColor color = QColor(125,125,125,125);
        painter->setBrush(color);
        painter->drawLine(mapFromScene(controlPoint_->pos()), QPoint(0,0));
        painter->drawEllipse(boundingRect());
        //For debugging purposes TODO: remove
        painter->drawText(QPointF(10,10),QString(QString::number(controlPoint_->vertexHandleIdx())));
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
    }
    return value;
}
