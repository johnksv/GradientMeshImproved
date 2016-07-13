#ifndef GMOPENGLWIDGET_H
#define GMOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QOpenGLFunctions_1_0>
#include "GUILogic/meshhandler.h"

class GMOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions_1_0
{
    Q_OBJECT

public:
    GMOpenGLWidget(GUILogic::MeshHandler *meshHandler, QWidget *parent = nullptr);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    GUILogic::MeshHandler *meshHandler;
    // use resizeGL to update.
    GLdouble glWidth, glHeight;
};

#endif // GMOPENGLWIDGET_H
