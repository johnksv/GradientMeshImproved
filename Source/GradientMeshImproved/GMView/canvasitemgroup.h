#ifndef CANVASITEMGROUP_H
#define CANVASITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QStandardItem>
#include <QString>
#include "canvasitemline.h"
#include "canvasitemface.h"
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
    void removeFromGroup(QGraphicsItem *item);
    void resetPointsHighlighted();
    friend class GMCanvas;

private:
    vector<CanvasItemPoint*> points;
    vector<CanvasItemLine*> lines;

};

} // end of namespace GMView

#endif // CANVASITEMGROUP_H
