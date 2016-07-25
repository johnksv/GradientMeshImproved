#include "undoweight.h"

UndoWeight::UndoWeight(QUndoCommand *parent) : QUndoCommand(parent)
{
    setText("Weight changed on vertex");
}

void UndoWeight::redo()
{

}

void UndoWeight::undo()
{

}
