#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "canvasitem.h"
#include "openglwidget.h"

enum class drawModeCanvas{
    vertices, vertAndEdge, edge, faces
};

class GMCanvas : public QGraphicsScene
{
    Q_OBJECT
public:
    GMCanvas(QObject * parent = 0);

    /*! Handel action from File Dialog related to the mesh, such as import and export.
     * \param location the location of the file to import/export
     * \param import if the mesh should be imported or exported. True for import. False for export.
     */
    void handleFileDialog(QString location, bool import);

    void setRenderingMode(int);
    void setDrawingMode(drawModeCanvas);

protected:
  //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    std::vector<GMCanvasItem*> item_points;
    GMCanvasItem *prevCanvasItem;
    GMOpenGLWidget *opengl;

    GUILogic::MeshHandler meshHandler;

    //0 for verticies only, 3 for "full" for full rendering
    unsigned char renderingMode = 3;

    /* 0 for only vertices. 1 for vertices and edges
     * TODO: implement: 2 for only edges. 3 for connecting verticies to faces
     */
    drawModeCanvas drawMode = drawModeCanvas::vertices;

    QPoint oldMousePos;
};

#endif // GMCANVAS_H
