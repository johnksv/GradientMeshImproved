#include "canvaspointconstraint.h"
#include <QPainter>


CanvasPointConstraint::CanvasPointConstraint(CanvasItemPoint *controlPoint, CanvasItemLine *edge, QGraphicsItem *parent)
    : controlPoint_(controlPoint), edge_(edge), QGraphicsItem(parent)
{
    setZValue(2);
    setFlags(ItemSendsScenePositionChanges);
}

QRectF CanvasPointConstraint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
}

void CanvasPointConstraint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = QColor(125,125,125,125);
    painter->setBrush(color);
    painter->drawEllipse(boundingRect());
    //For debugging purposes TODO: remove
    painter->drawText(QPointF(10,10),QString(QString::number(controlPoint_->vertexHandleIdx())));
}

QPainterPath CanvasPointConstraint::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

QVector3D CanvasPointConstraint::gradientConstraintVec()
{
    return gradientConstraintVec_;
}

QVariant CanvasPointConstraint::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange)
    {
        //TODO: Snap to line

    }
    return value;
}
