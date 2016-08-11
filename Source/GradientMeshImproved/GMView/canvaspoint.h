#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>
#include <QRectF>
#include "GUILogic/meshhandler.h"

namespace GMView{

class CanvasItemPoint : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    CanvasItemPoint(QColor color = QColor(Qt::black), QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    QColor color() const;
    void setColor(QColor color);

    int vertexHandleIdx() const;
    void setVertexHandleIdx(int idx);

    void setRadius(int);

    bool isHighlighted() const;
    void setHighlighted(bool highlighted);

    bool discontinuous() const;
    void setDiscontinuous(bool value, QGraphicsItem *edgeItem);

    //TODO better way to avoid header include collision
    //Returns the control point. Had to cast it at CanvasItemPoint due to header collision..
    //Can be casted to CanvasPointConstraint*
    QGraphicsItem *controlPoint(QGraphicsItem *_edge);


protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QColor color_;
    double weight_;
    int radius_ = 5;
    bool hovered_ = false;
    bool highlighted_ = false;
	int vertexHandleIdx_;
    bool discontinuous_ = false;
    //Must be of type QGraphicsItem to avoid header collision.
    vector<QGraphicsItem*> canvasPointDiscontinuedChildren_;
};

} // end of namespace GMView

#endif // GMCANVASITEM_H
