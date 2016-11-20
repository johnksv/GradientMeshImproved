#include "guichange.h"

using namespace GMView::undoCommands;

GUIChange::GUIChange(GMView::GMCanvas *scene,  QString meshOld, QUndoCommand *parent) : scene_(scene), meshOld_(meshOld)
{
    setText("GUI change");
}

void GUIChange::undo()
{
    //Import file with stringstream
    stringstream stream(meshOld_.toStdString());
    scene_->clearAllCurrLayer();
    scene_->currentMeshHandler()->importMesh(stream);
    scene_->constructGuiFromMeshHandler();
}
