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
    GMOpenGLWidget(vector<GUILogic::MeshHandler *> *meshHandlers, QWidget *parent = nullptr);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    vector<GUILogic::MeshHandler *> *meshHandlers;
    // use resizeGL to update.
    GLdouble glWidth, glHeight;
};

#endif // GMOPENGLWIDGET_H
