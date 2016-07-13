#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "canvaspoint.h"
#include "canvasitemline.h"
#include "canvasitemface.h"
#include "openglwidget.h"

enum class drawModeCanvas{
    move, vertAndEdge, edge, faces
};

class GMCanvas : public QGraphicsScene
{
    Q_OBJECT
public:
    GMCanvas(QObject * parent = 0);

    void clearAll();

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

public slots:
    void setDrawColorVertex(QColor pointColor);

protected:
  //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    vector<CanvasItemPoint*> item_points;
    vector<CanvasItemPoint*> items_selected;
    vector<CanvasItemLine*> item_lines;
    vector<CanvasItemFace*> item_faces;
    GMOpenGLWidget *opengl;
    QColor pointColor_;

    GUILogic::MeshHandler meshHandler_;

    //0 for verticies only, 3 for "full" for full rendering
    unsigned char renderingMode_ = 3;

    /* 0 for move and select. 1 for vertices and edges
     * TODO: implement: 2 for only edges. 3 for connecting verticies to faces
     */
    drawModeCanvas drawMode_ = drawModeCanvas::vertAndEdge;

    /*! Adds an CanvasItemPoint to this GMCanvas graphics scene, and updates the necessary dependencies.
     * \param CanvasItemPoint position of the item point
     */
    void handleMousePressVert(QGraphicsSceneMouseEvent *mouseEvent);
    void addItemPoint(CanvasItemPoint *item);
    void makeFace();
};

#endif // GMCANVAS_H
