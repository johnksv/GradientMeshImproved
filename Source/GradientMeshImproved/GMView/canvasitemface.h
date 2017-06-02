#ifndef CANVASITEMFACE_H
#define CANVASITEMFACE_H

#include <QGraphicsItem>
#include <QPolygon>
#include "canvasitemline.h"

namespace GMView{

class CanvasItemFace : public QGraphicsItem
{
public:
    CanvasItemFace(QGraphicsItem *parent = Q_NULLPTR);
    CanvasItemFace(QGraphicsItem *parent, int faceIdx, unsigned char layerId);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addCanvasEdge(vector<CanvasItemLine *> &edges);

    int faceIdx() const;

    void setFaceIdx(int faceIdx);

    vector<CanvasItemLine *> &edgesInFace();

    unsigned char layerId() const;

    void setLayerId(unsigned char layerId);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    vector<CanvasItemLine*> edgesInFace_;
    vector<QPointF> subdivedLines_;
    vector<bool> reverseEdge_;
    int faceIdx_;
    unsigned char layerId_;
};

}// end of namespace GMView

#endif // CANVASITEMFACE_H
