#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QGraphicsScene>
#include "openglwidget.h"
#include "canvasitemgroup.h"


/*! Namespace for handling user input
 * and representing data to GUI-part of application.
 */
namespace GMView {

enum class drawModeCanvas{
    move, lineTool
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

    void clearAll();
    void resetLineStartEnd();

    /*! Handel action from File Dialog related to the mesh, such as import and export.
     * \param location the location of the file to import/export
     * \param import if the mesh should be imported or exported. True for import. False for export.
     */
    void handleFileDialog(QString location, bool import);

    void setDrawingMode(drawModeCanvas);
    void setRenderingMode(renderModeCanvas);
    void setActiveLayer(unsigned char index);
    void setDrawColorVertex(QColor pointColor);
    void setRenderConstraintHandlers(bool value);

    drawModeCanvas drawingMode() const;
    renderModeCanvas renderMode() const;
    bool renderConstraintHandlers() const;


    vector<CanvasItemGroup *> layers();
    vector<GUILogic::MeshHandler *> *meshHandlers();
    GUILogic::MeshHandler *currentMeshHandler();

    void addLayer(QString name);
    void deleteLayer(int index);
    void toogleLayerVisibility(int index);

    void drawOpenGlOnCanvas(bool drawOnCanvas);

    //Temp. Should be moved when split-view
    void prepareRendering();



protected:
  //void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:
    vector<GUILogic::MeshHandler*> meshHandlers_;
    vector<CanvasItemGroup*> layers_;
    GMOpenGLWidget *opengl_;
    QColor pointColor_;
    CanvasItemPoint *lineStartPoint_ = nullptr, *lineEndPoint_ = nullptr;

    vector<CanvasItemPoint*> vertsToAddFace_;

    //Referces to the index in the layers vector. 0 index is first element
    unsigned char currLayerIndex_ = 0;

    drawModeCanvas drawMode_ = drawModeCanvas::lineTool;
    renderModeCanvas renderingMode_ =  renderModeCanvas::fullRender;
    bool renderConstraintHandlers_;

    /*! Adds an CanvasItemPoint to this GMCanvas graphics scene, and updates the necessary dependencies.
     * \param CanvasItemPoint position of the item point
     */
    void mouseLineTool(QGraphicsSceneMouseEvent *event);
    void addControlPoint(CanvasItemPoint *item);
    void showMessage(QString message, bool eraseLastVertToAddFace = false);
};

} // end of namespace GUIView

#endif // GMCANVAS_H
