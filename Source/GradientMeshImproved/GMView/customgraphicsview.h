#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget *parent = nullptr);

protected:
    virtual void wheelEvent(QWheelEvent *event);
private:
    double scaleFactor = 1.10;
};

#endif // CUSTOMGRAPHICSVIEW_H
