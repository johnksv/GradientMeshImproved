#include "canvaspointconstraint.h"
#include <QDebug>
#include <QPainter>
#include <QVariant>
#include <QLineF>
#include <QtMath>
#include <QApplication>

CanvasPointConstraint::CanvasPointConstraint(CanvasItemPoint *controlPoint, CanvasItemLine *edge, QGraphicsItem *parent)
    : controlPoint_(controlPoint), edge_(edge), QGraphicsItem(parent)
{
    setZValue(2);
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges);
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
        //May want to change the position differently depening on input type?
        if(QApplication::mouseButtons() == Qt::LeftButton)
        {
            QPointF *newPoint = &value.toPointF();
            QLineF startToNewLine(*newPoint, edge_->line().p1());

            /*Snaps to the point that intersects between the orgianl line,
            and the normal of the line from the new point and the lines startpoint */
            //Known bug: If startToNewLine -> startpoint
            startToNewLine.normalVector().intersect( edge_->line(), newPoint);
            return *newPoint;
        }
    }
    return value;
}
