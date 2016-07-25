#ifndef UNDOWEIGHT_H
#define UNDOWEIGHT_H
#include <QUndoCommand>

class UndoWeight : public QUndoCommand
{
public:
    UndoWeight(QUndoCommand *parent = nullptr);
    virtual void redo() override;
    virtual void undo() override;
};

#endif // UNDOWEIGHT_H
