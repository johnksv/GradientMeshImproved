#include "gmcanvas.h"
#include <QMouseEvent>
#include <QDebug>

GMCanvas::GMCanvas(QWidget *parent) :
    QOpenGLWidget{parent}
{

}

void GMCanvas::initializeGL()
{
    // initialise an OpenGL context
    initializeOpenGLFunctions();
    glWidth = 1200;
    glHeight = 1000;
}

void GMCanvas::resizeGL(int, int)
{
    glOrtho(0, glWidth, 0, glHeight, -1.0, 1.0);
}

void GMCanvas::setRenderingMode(int mode){
    renderingMode = mode;
}

void GMCanvas::paintGL()
{    
    // Qt painter, create object on stack
    // for automatic garbage collection (no need to call end())
    QPainter qPainter;

    /******* Start painting with OpenGL ***********/
    qPainter.begin(this);
    qPainter.beginNativePainting();

    // set background colour and clear framebuffer
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw stuff
    if(renderingMode == 3){
        meshHandler.drawGLMesh(this);
    }

    /******* Start painting with Qt ***********/
    qPainter.endNativePainting();

    // draw other stuff on top of the OpenGL painting
    if(renderingMode == 0){
       vector<vector<float>> points = meshHandler.getVertices();
       for(int i = 0; i < points.size(); i++){
           qPainter.drawPoint(points[i][0],points[i][1]);
       }
    }
}

void GMCanvas::handleFileDialog(QString location, bool import){
    if(import){

    }else{
        meshHandler.saveGuiMeshOff(location);
    }
}

void GMCanvas::mousePressEvent(QMouseEvent* event){
    int x = event->x();
    int y = event->y();
    if(event->button() == Qt::RightButton){
        meshHandler.makeFace();
        qDebug() << "RightClick";
    }
    meshHandler.addVertexFromPoint(event->pos());
    qDebug() << "x,y : " << x <<" , "<< y ;
}

void GMCanvas::mouseMoveEvent(QMouseEvent *event){
    int x = event->x();
    int y = event->y();


    qDebug() << "MouseMove: x,y : " << x <<" , "<< y ;
}
