#ifndef UNDOCOLORCOMMAND_H
#define UNDOCOLORCOMMAND_H

#include <QUndoCommand>
#include <QColor>


class UndoColor : public QUndoCommand
{
public:
    UndoColor(int vertexIdx, QColor newColor, QColor oldColor, QUndoCommand *parent = nullptr);
    virtual void redo() override;
    virtual void undo() override;

private:
    int vertexIdx_;
    QColor newColor_;
    QColor oldColor_;
};

#endif // UNDOCOLORCOMMAND_H
