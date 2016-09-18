#ifndef CANVASITEMGROUP_H
#define CANVASITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QStandardItem>
#include <QString>
#include "canvasitemline.h"
#include "canvaspointconstraint.h"

namespace GMView{


class CanvasItemGroup : public QGraphicsItem, public QStandardItem
{
public:
    CanvasItemGroup(QString layername, QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    void addToGroup(QGraphicsItem *item);
    void resetPointsHighlighted();
    friend class GMCanvas;

private:
    QGraphicsItemGroup points;
    QGraphicsItemGroup lines;

};

} // end of namespace GMView

#endif // CANVASITEMGROUP_H
