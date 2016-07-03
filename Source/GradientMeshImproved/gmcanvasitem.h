#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H
#include <QGraphicsItem>
#include <QRectF>
#include "GUILogic/meshhandler.h"

enum class drawModeCanv{
    vertices, vertAndEdge, edge, faces
};

class GMCanvasItem :public QGraphicsItem
{
public:

    GMCanvasItem(QGraphicsItem *parent = Q_NULLPTR, QPointF pos = QPointF(0,0));
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setRadius(int);
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    GUILogic::MeshHandler meshHandler;

    //0 for verticies only, 3 for "full" for full rendering
    unsigned char renderingMode = 3;
    /* 0 for only vertices. 1 for vertices and edges
     * TODO: implement: 2 for only edges. 3 for connecting verticies to faces
     */
    drawModeCanv drawMode = drawModeCanv::vertices;

    QPointF  position, oldMousePos;
    int radius = 5;


};

#endif // GMCANVASITEM_H
