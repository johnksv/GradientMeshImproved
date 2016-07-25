#ifndef MOVEVERTEX_H
#define MOVEVERTEX_H

#include <QUndoCommand>

class moveVertex : public QUndoCommand
{
public:
    moveVertex(QUndoCommand *parent = nullptr);
    virtual void redo() override;
    virtual void undo() override;
};

#endif // MOVEVERTEX_H
