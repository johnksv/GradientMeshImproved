#ifndef CANVASITEMLINE_H
#define CANVASITEMLINE_H

#include <QGraphicsLineItem>
#include "canvaspoint.h"
#include <QMenu>

namespace GMView{

class CanvasItemLine : public QGraphicsLineItem
{

public:
    CanvasItemLine(CanvasItemPoint* startPoint, CanvasItemPoint* endPoint, QGraphicsItem *parent = Q_NULLPTR);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    bool operator ==(const CanvasItemLine &lineA);

    QColor color();
    void setColor(QColor &color = QColor(0,0,0));

	CanvasItemPoint *startPoint();
	CanvasItemPoint *endPoint();

    void updateSubdivisonCurve();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

private:
    CanvasItemPoint *startPoint_, *endPoint_;

    QColor color_;
    bool discontinuous_ = false;
    QMenu contextMenu_;
    vector<QPointF> subdividedCurve_;


    vector<QPointF> subdCurve(vector<QPointF> spline, int steps = 3, bool closed = false);
};

} // end of namespace GUIView

#endif // CANVASITEMLINE_H
