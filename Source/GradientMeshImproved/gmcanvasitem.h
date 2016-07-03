#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>

class GMCanvasItem :public QGraphicsItem
{
public:
    GMCanvasItem(QGraphicsItem *parent = Q_NULLPTR);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

};

#endif // GMCANVASITEM_H
