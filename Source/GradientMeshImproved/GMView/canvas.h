#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "canvasitemgroup.h"


/*! Namespace for handling user input
 * and representing data to GUI-part of application.
 */
namespace GMView {

enum class drawModeCanvas{
    move, lineTool, circleTool, collapseEdge, insertVert
};
enum class renderModeCanvas{
    fullRender
};

class GMCanvas : public QGraphicsScene
{
    Q_OBJECT

public:
    GMCanvas(QObject * parent = 0);
    ~GMCanvas();

    void clear();
    void clearAllCurrLayer(bool clearMeshHandler = true);
    void resetToBeFaceVector();
    void resetLineStartEnd();

    /*! Handel action from File Dialog related to the mesh, such as import and export.
     * \param location the location of the file to import/export
     * \param import if the mesh should be imported or exported. True for import. False for export.
     */
    void handleFileDialog(QString location, bool import);
	void importFile(QString location);
    //If import = false, then image will be removed
    void handleImageFileDialog(QString location, bool import);
    QGraphicsPixmapItem *imageItem();

    void setDrawingMode(drawModeCanvas);
    void setRenderingMode(renderModeCanvas);
    void setActiveLayer(unsigned char index);
    void setDrawColorVertex(QColor pointColor);
    void setRenderConstraintHandlers(bool value);
    void setRenderVertsEdges(bool renderVertsEdges);
    void setRenderAuto(bool renderAutoUpdate);

    drawModeCanvas drawingMode() const;
    renderModeCanvas renderMode() const;
    bool renderConstraintHandlers() const;



    vector<CanvasItemGroup *> layers();
    CanvasItemGroup *currentLayer();
    vector<GUILogic::MeshHandler *> *meshHandlers();
    GUILogic::MeshHandler *currentMeshHandler();
    vector<GUILogic::MeshHandler *> *multiResMeshHandlers();

    void addLayer(QString name);
    void deleteLayer(int index);
    void toogleLayerVisibility(int index);

    void drawOpenGlOnCanvas(bool drawOnCanvas);

    //Temp. Should be moved when split-view
    void prepareRendering();

	void multiResFirstStepMesh();
    void resetMultiResMesh();

    void constructGuiFromMeshHandler(bool fromMultiRes = false, int indexOfMultiResLayer = 0);


    /* Calls appropriate functions to render the changed mesh.
     */
    void autoRenderOnMeshChanged();

protected:
    //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    vector<GUILogic::MeshHandler*> meshHandlers_;
    vector<CanvasItemGroup*> layers_;


    vector<GUILogic::MeshHandler*> multiRes_meshHandlers_;
    vector<CanvasItemGroup*> multiRes_layers_;

    QGraphicsProxyWidget *opengl_;
    QColor pointColor_;
    CanvasItemPoint *lineStartPoint_ = nullptr, *lineEndPoint_ = nullptr;

    vector<CanvasItemPoint*> vertsToAddFace_;

    //Referces to the index in the layers vector. 0 index is first element
    unsigned char currLayerIndex_ = 0;

    drawModeCanvas drawMode_ = drawModeCanvas::lineTool;
    renderModeCanvas renderingMode_ =  renderModeCanvas::fullRender;
    bool renderConstraintHandlers_;
    bool renderAutoUpdate_ = true;

    QGraphicsPixmapItem *imageItem_ = nullptr;

    /*! Adds an CanvasItemPoint to this GMCanvas graphics scene, and updates the necessary dependencies.
     * \param CanvasItemPoint position of the item point
     */
    void mouseLineTool(QGraphicsSceneMouseEvent *event);
    void mouseCircleTool(QGraphicsSceneMouseEvent *event);
    void addItemLine(CanvasItemPoint* itemPoint, CanvasItemPoint* collidePoint, bool collide);

    void addControlPoint(CanvasItemPoint *item);
    void showMessage(QString message, bool eraseLastVertToAddFace = false);
    /*!
     * Helper function for mouse tools.
     */
    QGraphicsItem* findCollideWithPoint(CanvasItemPoint* itemPoint);

    void mouseCollapseEdge(QGraphicsSceneMouseEvent *mouseEvent);

    void mouseInsertVertOnEdge(QGraphicsSceneMouseEvent *event);
    
    void updateVertexConstraints();

    void addEdgesToCanvasFace(const vector<int>& vertsToAddFaceIdx);

    CanvasItemLine *edgeBetweenPoints(const int &vertIdPoint1,const int &vertIdPoint2);

    void initGMCanvas();

signals:
    void GUIMeshChanged();
};

} // end of namespace GMView

#endif // GMCANVAS_H
