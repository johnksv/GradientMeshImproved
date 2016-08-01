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

    QColor color();
    double weight();
    void setColor(QColor color);
    void setWeight(double);

	int vertexHandleIdx();
	void setVertexHandleIdx(int idx);

    void setRadius(int);

    bool isHighlighted();
    void setHighlighted(bool highlighted);

	bool catmullInterpolation();
	void setCatmullInterpolation(bool value);

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
	bool catmullInterpolation_ = false;
};

} // end of namespace GUIView

#endif // GMCANVASITEM_H
