#include <QtTest>
#include <QDebug>
#include "test/testcanvas.h"

using namespace testclasses;

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
     qDebug() << "Debugging test";

    TestCanvas canvas;

    return QTest::qExec(&canvas, argc, argv);
}
