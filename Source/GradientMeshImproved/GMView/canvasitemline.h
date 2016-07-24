#ifndef CANVASITEMLINE_H
#define CANVASITEMLINE_H

#include <QGraphicsLineItem>
#include "canvaspoint.h"

class CanvasItemLine : public QGraphicsLineItem
{

public:
    CanvasItemLine(CanvasItemPoint* startPoint, CanvasItemPoint* endPoint, QGraphicsItem *parent = Q_NULLPTR);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    bool operator ==(const CanvasItemLine &lineA);

    int edgeHandleIdx();
    void setEdgeHandleIdx(int idx);

private:
    CanvasItemPoint *startPoint, *endPoint;
    int edgeHandleIdx_;
};

#endif // CANVASITEMLINE_H
