#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>
#include <QRectF>
#include "GUILogic/meshhandler.h"

namespace GMView{

class CanvasItemPoint : public QGraphicsItem
{
public:
    CanvasItemPoint(QColor color = QColor(Qt::black), QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    QColor color() const;
    void setColor(QColor color, bool callAutoRender = true);

    int vertexHandleIdx() const;
    void setVertexHandleIdx(int idx);

    double radius() const;
    void setRadius(int);

    bool isHighlighted() const;
    void setHighlighted(bool highlighted);

    //TODO better way to avoid header include collision
    //Returns the constraint point associated with the edge
    //Can be casted to CanvasPointConstraint*
    QGraphicsItem *constraintPoint(QGraphicsItem *_edge);


protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QColor color_;
    //The radius is calculated in paint
    double radius_ = 0.01;
    bool highlighted_ = false;
	int vertexHandleIdx_;
    bool discontinuous_ = false;
};

} // end of namespace GMView

#endif // GMCANVASITEM_H
