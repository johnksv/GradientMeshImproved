#include "testcanvas.h"
#include "GMView/canvas.h"
#include "GMView/customgraphicsview.h"
#include "GUILogic/meshhandler.h"
#include <stdio.h>

using namespace QTest;

GMView::CustomGraphicsView *customGraphicsView;
QWidget *view;
GMView::GMCanvas *canvas;
GUILogic::MeshHandler *meshHandler;


void testclasses::TestCanvas::init()
{
    customGraphicsView = new GMView::CustomGraphicsView();
    canvas = new GMView::GMCanvas();
    customGraphicsView->setScene(canvas);

    view = customGraphicsView->viewport();

    meshHandler = canvas->meshHandlers().front();
}

void testclasses::TestCanvas::cleanup()
{
    delete canvas;
    delete customGraphicsView;
    std::remove("test.off");
}

void testclasses::TestCanvas::test_add_first_face()
{
    vector<QPoint> points{QPoint(0,0), QPoint(0,10), QPoint(10, 10), QPoint(10, 0)};
    for(int i = 0; i < points.size(); i++){
        mouseClick(view, Qt::LeftButton, Qt::NoModifier, points.at(i));
    }
    mouseClick(view, Qt::LeftButton, Qt::NoModifier, points.front());

    QVERIFY(meshHandler->numberOfFaces() == 1);
}

void testclasses::TestCanvas::test_message_when_new_control_point_is_not_connected_to_existing_face()
{
    test_add_first_face();

    mouseClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(20,20));

    QVERIFY(meshHandler->numberOfFaces() == 1);
}
