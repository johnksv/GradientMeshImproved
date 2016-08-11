#ifndef CANVASPOINTCONSTRAINT_H
#define CANVASPOINTCONSTRAINT_H

#include "canvasitemline.h"
#include "canvaspointdiscontinued.h"

namespace GMView{

class CanvasPointConstraint : public QGraphicsItem
{
public:
    CanvasPointConstraint(QGraphicsItem *controlPoint, CanvasItemLine* edge, QGraphicsItem *parent = Q_NULLPTR);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    //Of type CanvasItemPoint or CanvasPointDiscontinued
    QGraphicsItem *controlPoint();
    CanvasItemLine *edge();

    void setInactive(bool value);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;

private:
    CanvasItemPoint *controlPoint_ = nullptr;
    CanvasPointDiscontinued *controlPointDis_ = nullptr;
    CanvasItemLine *edge_;

    bool inactive_ = false;

    int radius_ = 3;
};

} // end of namespace GMView

#endif // CANVASPOINTCONSTRAINT_H
