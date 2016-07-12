#ifndef CANVASITEMFACE_H
#define CANVASITEMFACE_H

#include <QGraphicsItem>
#include <QPolygon>
#include "canvaspoint.h"

class CanvasItemFace :public QGraphicsItem
{
public:
    CanvasItemFace(QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addCanvasPoint(CanvasItemPoint*);

private:
    vector<CanvasItemPoint*> pointsInFace_;
};

#endif // CANVASITEMFACE_H
