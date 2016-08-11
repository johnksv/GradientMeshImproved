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

    bool isDiscontinuous() const;
    void setDiscontinuous(bool value, QGraphicsItem *edgeItem);

    vector<QGraphicsItem*> discontinuedChildren();

    //TODO better way to avoid header include collision
    //Returns the constraint point associated with the edge
    //Can be casted to CanvasPointConstraint*
    QGraphicsItem *constraintPoint(QGraphicsItem *_edge);


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

    /*Must be of type QGraphicsItem to avoid header collision.
     * The first element should always have the same vertexIDX as this point.
     */
    vector<QGraphicsItem*> discontinuedChildren_;
};

} // end of namespace GMView

#endif // GMCANVASITEM_H
