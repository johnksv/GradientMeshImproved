#ifndef GMCANVASITEM_H
#define GMCANVASITEM_H

#include <QGraphicsScene>

class GMCanvas : public QGraphicsScene
{
public:
    GMCanvas(QObject * parent = 0);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) override;

private:


};

#endif // GMCANVASITEM_H
