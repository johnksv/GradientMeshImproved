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
    QPainter qPainter;
    /******* Start painting with OpenGL ***********/
    qPainter.begin(this);
    qPainter.scale(scale,scale);
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
        qPainter.setBrush( Qt::black );
        qPainter.setPen( Qt::black );
       vector<vector<float>> points = meshHandler.getVertices();
       for(int i = 0; i < points.size(); i++){
           qPainter.drawEllipse(QPointF(points[i][0],points[i][1]), 5,5);
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
    paintGL();
    update();
    qDebug() << "x,y : " << x <<" , "<< y ;
}

void GMCanvas::mouseMoveEvent(QMouseEvent *event){
    int x = event->x();
    int y = event->y();
}

void GMCanvas::wheelEvent(QWheelEvent *event){
    //TODO: Smooth scaling and zoom on middel of canvas
    //TODO: "World" pixel-values

    // positive value rotated away from the user
    int deltaY = event->angleDelta().y();
    //Check if zooming
    if(deltaY > 0){
        scale += deltaY/120;
    }else{
        scale += deltaY/120;
    }
    update();
}
