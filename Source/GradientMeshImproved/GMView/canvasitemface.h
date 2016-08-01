#ifndef CANVASITEMFACE_H
#define CANVASITEMFACE_H

#include <QGraphicsItem>
#include <QPolygon>
#include "canvaspoint.h"

namespace GMView {

class CanvasItemFace :public QGraphicsItem
{
public:
    Q_DECL_DEPRECATED CanvasItemFace(QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addCanvasPoint(CanvasItemPoint*);

private:
    vector<CanvasItemPoint*> pointsInFace_;
};

} // end of namespace GUIView

#endif // CANVASITEMFACE_H
