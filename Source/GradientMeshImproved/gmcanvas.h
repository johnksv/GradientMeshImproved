#ifndef GMCANVAS_H
#define GMCANVAS_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QOpenGLFunctions_1_0>
#include "GUILogic/meshhandler.h"

/*! \brief GMCanvas.
 *         Controller class for managing the canvas
 *         which is used to draw the gradient mesh (GM).
 *
 *  Usage: MainWindow class.
 *  Dependencies: GUILogic namespace.
 *
 * Version: 0 (class in development)
 */

class GMCanvas : public QOpenGLWidget, public QOpenGLFunctions_1_0
{
    Q_OBJECT

public:
    GMCanvas(QWidget *parent = nullptr);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;
protected:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

private:
    GUILogic::MeshHandler meshHandler;

    // width and height of openGL canvas.
    // use resizeGL to update.
    GLdouble glWidth, glHeight;
};

#endif // GMCANVAS_H
