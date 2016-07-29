#ifndef CANVASITEMLINE_H
#define CANVASITEMLINE_H

#include <QGraphicsLineItem>
#include "canvaspoint.h"
#include <QMenu>

class CanvasItemLine : public QGraphicsLineItem
{

public:
    CanvasItemLine(CanvasItemPoint* startPoint, CanvasItemPoint* endPoint, QGraphicsItem *parent = Q_NULLPTR);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    bool operator ==(const CanvasItemLine &lineA);

    int edgeHandleIdx();
    void setEdgeHandleIdx(int idx);

    QColor color();
    void setColor(QColor &color = QColor(0,0,0));

	CanvasItemPoint *startPoint();
	CanvasItemPoint *endPoint();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

private:
    CanvasItemPoint *startPoint_, *endPoint_;
    int edgeHandleIdx_;
    QColor color_;
    bool discontinuous_ = false;
    QMenu contextMenu_;
};

#endif // CANVASITEMLINE_H
