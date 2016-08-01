#ifndef CANVASPOINTCONSTRAINT_H
#define CANVASPOINTCONSTRAINT_H

#include "canvasitemline.h"

namespace GMView{

class CanvasPointConstraint : public QGraphicsItem
{
public:
    CanvasPointConstraint(CanvasItemPoint *controlPoint, CanvasItemLine* edge, QGraphicsItem *parent = Q_NULLPTR);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    QVector3D gradientConstraintVec();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;

private:
    CanvasItemPoint *controlPoint_;
    CanvasItemLine *edge_;

    QVector3D gradientConstraintVec_;
    int radius_ = 3;
};

} // end of namespace GUIView

#endif // CANVASPOINTCONSTRAINT_H
