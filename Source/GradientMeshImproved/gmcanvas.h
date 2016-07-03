#ifndef GMCANVAS_H
#define GMCANVAS_H

#include "gmcanvasitem.h"
#include "gmopenglwidget.h"
#include <QGraphicsScene>
#include <QOpenGLFunctions_1_0>

class GMCanvas : public QGraphicsScene
{
    Q_OBJECT
public:
    GMCanvas(QObject * parent = 0);
    ~GMCanvas();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    GMCanvasItem *mainCanvasItem;
    GMOpenGLWidget *opengl;
};

#endif // GMCANVAS_H
