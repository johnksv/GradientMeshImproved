#ifndef GMOPENGLWIDGET_H
#define GMOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QOpenGLFunctions_1_0>
#include "GUILogic/meshhandler.h"
#include "canvas.h"

namespace GMView{

class GMOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions_1_0
{
    Q_OBJECT

public:
    GMOpenGLWidget(QWidget *parent = nullptr);
    GMOpenGLWidget(GMCanvas *scene, QWidget *parent = nullptr);
    void setScene(GMCanvas* scene);
    void setRenderMode(renderModeOpenGL mode);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;

private:
    GMCanvas *scene_;
    renderModeOpenGL renderMode_ = renderModeOpenGL::viewportRender;
    // use resizeGL to update.
};

} // end of namespace GMView

#endif // GMOPENGLWIDGET_H
