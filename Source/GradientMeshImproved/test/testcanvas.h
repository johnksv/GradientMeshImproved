#ifndef TESTCANVAS_H
#define TESTCANVAS_H

#include <QTest>

namespace testclasses {

class TestCanvas : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_add_first_face();
    void test_message_when_new_control_point_is_not_connected_to_existing_face();

};

}

#endif // TESTCANVAS_H
