#ifndef GMOPENGLWIDGET_H
#define GMOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QOpenGLFunctions_1_0>
#include "GUILogic/meshhandler.h"

namespace GMView{

class GMOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions_1_0
{
    Q_OBJECT

public:
    GMOpenGLWidget(QWidget *parent = nullptr);
    GMOpenGLWidget(vector<GUILogic::MeshHandler *> *meshHandlers_, vector<GUILogic::MeshHandler *> *multiResMeshHandlers, QWidget *parent = nullptr);
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;
    void setMeshHandlers(vector<GUILogic::MeshHandler *> *meshHandlers_);
    void setMultiResMeshHandlers(vector<GUILogic::MeshHandler *> *meshHandlers_);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    vector<GUILogic::MeshHandler *> *meshHandlers_;
    vector<GUILogic::MeshHandler *> *multiRes_meshHandlers_;
    // use resizeGL to update.
    GLdouble glWidth, glHeight;
};

} // end of namespace GMView

#endif // GMOPENGLWIDGET_H
