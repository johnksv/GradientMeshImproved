#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "canvaspoint.h"
#include "canvasitemline.h"
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

    /*! Updates the corresponding vertex in meshhandler with new information from the CanvasItemPoint.
     * This function is ment to be called from CanvasItemPoint methods.
     * \param mode the value to update. 0 for color, 1 for weight.
     */
    void updateVertexFromPoint(CanvasItemPoint *item,short mode);

protected:
  //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    vector<CanvasItemPoint*> item_points;
    vector<CanvasItemPoint*> items_selected;
    GMOpenGLWidget *opengl;

    GUILogic::MeshHandler meshHandler;

    //0 for verticies only, 3 for "full" for full rendering
    unsigned char renderingMode = 3;

    /* 0 for only vertices. 1 for vertices and edges
     * TODO: implement: 2 for only edges. 3 for connecting verticies to faces
     */
    drawModeCanvas drawMode = drawModeCanvas::vertices;

    QPoint oldMousePos;

    /*! Adds an CanvasItemPoint to this GMCanvas graphics scene, and updates the necessary dependencies.
     * \param CanvasItemPoint position of the item point
     */
    void handleMousePressVert(QGraphicsSceneMouseEvent *mouseEvent);
    void addItemPoint(CanvasItemPoint *item);
    void makeFace();
};

#endif // GMCANVAS_H
