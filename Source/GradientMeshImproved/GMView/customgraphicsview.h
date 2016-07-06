#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    double scaleFactor = 1.10;
};

#endif // CUSTOMGRAPHICSVIEW_H
