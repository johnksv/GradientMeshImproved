#ifndef GUICHANGE_H
#define GUICHANGE_H


#include <QUndoCommand>
#include "GMView/canvas.h"

namespace GMView
{
    namespace undoCommands
    {
        class GUIChange : public QUndoCommand
        {
        public:
            GUIChange(GMCanvas *scene, QString meshOld, QString meshNew, QUndoCommand *parent = nullptr);
            virtual void undo() override;
            virtual void redo() override;

        private:
            GMCanvas *scene_;
            QString meshOld_;
            QString meshNew_;
            bool firstRun_ = true;
        };
    } //End namespace undoCommands
} //End namespace GMView

#endif // GUICHANGE_H
