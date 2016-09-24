#ifndef CANVASITEMFACE_H
#define CANVASITEMFACE_H

#include <QGraphicsItem>
#include <QPolygon>
#include "canvasitemline.h"

namespace GMView{

class CanvasItemFace : public QGraphicsItem
{
public:
    CanvasItemFace(QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addCanvasEdge(CanvasItemLine* edge);

private:
    vector<CanvasItemLine*> edgesInFace_;

};

}// end of namespace GMView

#endif // CANVASITEMFACE_H
