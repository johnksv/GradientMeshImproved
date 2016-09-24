#ifndef CANVASITEMGROUP_H
#define CANVASITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QStandardItem>
#include <QString>
#include "canvasitemface.h"
#include "canvaspointconstraint.h"

namespace GMView{

class CanvasItemContainer : public QGraphicsItem
{
public:
    CanvasItemContainer();
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
};

class CanvasItemGroup : public QGraphicsItem, public QStandardItem
{
public:
    CanvasItemGroup(QString layername, QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    void addToGroup(QGraphicsItem *item);
    void resetPointsHighlighted();
    CanvasItemContainer &points();
    CanvasItemContainer &lines();
    friend class GMCanvas;

private:
    CanvasItemContainer points_;
    CanvasItemContainer lines_;
    vector<CanvasItemFace*> faces_;

};

} // end of namespace GMView

#endif // CANVASITEMGROUP_H
