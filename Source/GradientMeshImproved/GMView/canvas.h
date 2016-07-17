#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "openglwidget.h"
#include "canvasitemgroup.h"

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
    vector<CanvasItemGroup *> layers();
    void setActiveLayer(unsigned char index);
    void addLayer(QString name);
    void deleteLayer(int index);
    void toogleLayerVisibility(int index);
    //Move down -> move towards last element
    void layerMoveIndex(int index, bool moveDown);

    /*! Updates the corresponding vertex in meshhandler with new information from the CanvasItemPoint.
     * This function is ment to be called from CanvasItemPoint methods.
     * \param mode the value to update. 0 for color, 1 for weight.
     */
    void updateVertexFromPoint(CanvasItemPoint *item,short mode);

    //Temp. Should be moved when split-view
    void prepareRendering();

public slots:
    void setDrawColorVertex(QColor pointColor);

protected:
  //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    vector<GUILogic::MeshHandler*> meshHandlers_;
    vector<CanvasItemGroup*> layers_;
    GMOpenGLWidget *opengl_;
    QColor pointColor_;

    //Referces to the index in the layers vector. 0 index is first element
    unsigned char currLayerIndex_ = 0;


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
