#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QOpenGLFunctions_1_0>
#include "GUILogic/meshhandler.h"

enum class drawModeCanvas{
    vertices, vertAndEdge, edge, faces
};

/*! \brief GMCanvas.
 *         Controller class for managing the canvas
 *         which is used to draw the gradient mesh (GM).
 *
 *  Usage: MainWindow class.
 *  Dependencies: GUILogic namespace.
 *
 * Version: 0 (class in development)
 */

class GMCanvasOLD : public QOpenGLWidget, public QOpenGLFunctions_1_0
{
    Q_OBJECT

public:
    GMCanvasOLD(QWidget *parent = nullptr);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;
    void setRenderingMode(int);
    void setDrawingMode(drawModeCanvas);
    /*! Handel action from File Dialog related to the mesh, such as import and export.
     * \param location the location of the file to import/export
     * \param import if the mesh should be imported or exported. True for import. False for export.
     */
    void handleFileDialog(QString location, bool import);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    GUILogic::MeshHandler meshHandler;

    // width and height of openGL canvas.
    // use resizeGL to update.
    GLdouble glWidth, glHeight;

    //0 for verticies only, 3 for "full" for full rendering
    unsigned char renderingMode = 3;
    /* 0 for only vertices. 1 for vertices and edges
     * TODO: implement: 2 for only edges. 3 for connecting verticies to faces
     */
    drawModeCanvas drawMode = drawModeCanvas::vertices;

    QPoint prevVer, oldMousePos;

    qreal scale = 1;
};

#endif // GMCANVAS_H
