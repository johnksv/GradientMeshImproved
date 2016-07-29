#include "movevertex.h"

moveVertex::moveVertex(QUndoCommand *parent) : QUndoCommand(parent)
{
    setText("Moved vertex");
}

void moveVertex::redo()
{

}

void moveVertex::undo()
{

}
