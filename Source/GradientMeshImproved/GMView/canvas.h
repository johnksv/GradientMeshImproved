#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "canvasitemgroup.h"


/*! Namespace for handling user input
 * and representing data to GUI-part of application.
 */
namespace GMView {

enum class drawModeCanvas{
    move,
    lineTool,
    collapseEdge,
    insertVert,
    meshToolInsertion,
    rectangleTool,
    diffusionPoints
};

enum class renderModeOpenGL{
    viewportRender,
    fullSceneRender,
    boundingRect
};

extern bool drawCanvasItemFaces;

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

    void saveViewportToPNG(const QString &location);
    void saveLayerToOFF(const QString &location);
    void saveAllLayersToOFF(const QString &location);
    bool importFile(const QString &location, const bool firstOfMultipleFiles = false);
    void importFileClean(QString location);
    void importFileLayer(QString location);
    //If import = false, then image will be removed
    void handleImageFileDialog(QString location, bool import);
    QGraphicsPixmapItem *imageItem();

    void setDrawingMode(drawModeCanvas);
    void setActiveLayerBack();
    void setActiveLayer(unsigned char index);
    void setDrawColorVertex(QColor pointColor);
    void setRenderConstraintHandlers(bool value);
    void setRenderVertsEdges(bool renderVertsEdges);
    void setRenderAuto(bool renderAutoUpdate);
    void setRenderFace(bool renderFace);

    drawModeCanvas drawingMode() const;
    bool renderConstraintHandlers() const;

    void setLayerModel(QStandardItemModel* model);

    vector<CanvasItemGroup *> layers();
    CanvasItemGroup *currentLayer();
    vector<GUILogic::MeshHandler *> meshHandlers();
    GUILogic::MeshHandler *currentMeshHandler();
    vector<GUILogic::MeshHandler *> multiResMeshHandlers();

    void addLayer();
    void addLayer(QString name);
    void deleteLayer(int index);
    void moveLayerUp(int indexToMove);
    void moveLayerDown(int indexToMove);
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

    void resizeOpenGLWidget();

    void setItemPointColorFromImage();

    void newDocument();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    vector<std::pair<CanvasItemGroup*, GUILogic::MeshHandler*>> layersAndMeshHandlers_;
    vector<std::pair<CanvasItemGroup*, GUILogic::MeshHandler*>> multiResLayersAndMeshHandlers_;

    QGraphicsProxyWidget *opengl_;
    QColor pointColor_;
    CanvasItemPoint *lineStartPoint_ = nullptr, *lineEndPoint_ = nullptr;

    vector<CanvasItemPoint*> vertsToAddFace_;

    //Referces to the index in the layers vector. 0 index is first element
    unsigned char currLayerIndex_ = 0;

    drawModeCanvas drawMode_ = drawModeCanvas::lineTool;
    bool renderConstraintHandlers_;
    bool renderAutoUpdate_ = true;

    QGraphicsPixmapItem *imageItem_ = nullptr;
    QGraphicsRectItem *rectItem_ = nullptr;

    QStandardItemModel* layerModel_;


    /*! Adds an CanvasItemPoint to this GMCanvas graphics scene, and updates the necessary dependencies.
     * \param CanvasItemPoint position of the item point
     */
    void mouseLineTool(QGraphicsSceneMouseEvent *event);
    void addItemLine(CanvasItemPoint* itemPoint, CanvasItemPoint* collidePoint, bool collide);

    void addControlPoint(CanvasItemPoint *item);
    void showMessage(QString message, bool eraseLastVertToAddFace = false);
    /*!
     * Helper function for mouse tools.
     */
    QGraphicsItem* findCollideWithPoint(CanvasItemPoint* itemPoint);

    void mouseCollapseEdge(QGraphicsSceneMouseEvent *mouseEvent);

    void mouseInsertVertOnEdge(QGraphicsSceneMouseEvent *event);

    void mouseMeshToolInsertion(QGraphicsSceneMouseEvent *event);

    void mouseRectangleTool(QGraphicsSceneMouseEvent *event);
    
    void updateVertexConstraints();

    void addEdgesToCanvasFace(const vector<int>& vertsToAddFaceIdx, int faceIdx);

    CanvasItemLine *edgeBetweenPoints(const int &vertIdPoint1,const int &vertIdPoint2);

    void initGMCanvas();

    void resetLineToolData();
    
    bool addFaceToOpnMesh(vector<int> &vertsToAddFaceIdx, CanvasItemPoint *collidePoint);

    void addConstrainsForLine(CanvasItemPoint* fromPoint, CanvasItemPoint *toPoint, CanvasItemLine *line);

signals:
    void GUIMeshChanged();
};

} // end of namespace GMView

#endif // GMCANVAS_H
