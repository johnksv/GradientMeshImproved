#ifndef CANVASPOINTDISCONTINUED_H
#define CANVASPOINTDISCONTINUED_H

#include <QGraphicsItem>

namespace GMView{

class CanvasPointDiscontinued : public QGraphicsItem
{
public:
    CanvasPointDiscontinued(bool sameIdxAsParent,QGraphicsItem *parent = Q_NULLPTR);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    QColor color() const;
    void setColor(QColor &color);

    int vertexHandleIdx() const;
    void setVertexHandleIdx(int idx);

    bool sameIdxAsParent() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    int radius_ = 3;
    int vertexHandleIdx_;
    bool sameIdxAsParent_;
    QColor color_ =QColor(123,158,92);
};

} // end of namespace GMView
#endif // CANVASPOINTDISCONTINUED_H
