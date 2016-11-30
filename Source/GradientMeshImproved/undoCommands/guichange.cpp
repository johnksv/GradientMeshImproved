#include "guichange.h"

using namespace GMView::undoCommands;

GUIChange::GUIChange(GMView::GMCanvas *scene,  QString meshOld, QString meshNew, QUndoCommand *parent)
    : scene_(scene), meshOld_(meshOld), meshNew_(meshNew)
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

    //So the mesh is subdiveded right away
    scene_->autoRenderOnMeshChanged();
}

void GUIChange::redo()
{
    if(firstRun_)
    {
        firstRun_ = false;
        return;
    }

    //Import file with stringstream
    stringstream stream(meshNew_.toStdString());
    scene_->clearAllCurrLayer();
    scene_->currentMeshHandler()->importMesh(stream);
    scene_->constructGuiFromMeshHandler();

    //So the mesh is subdiveded right away
    scene_->autoRenderOnMeshChanged();
}
