#include "undocolor.h"

UndoColor::UndoColor(int vertexIdx, QColor newColor, QColor oldColor,  QUndoCommand *parent) : QUndoCommand(parent), vertexIdx_(vertexIdx)
{
    setText("Color change on vertex");
}

void UndoColor::redo()
{

}

void UndoColor::undo()
{

}
