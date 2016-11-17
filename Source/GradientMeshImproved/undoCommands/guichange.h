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
            GUIChange(GMCanvas *scene, QString meshOld, QUndoCommand *parent = nullptr);
            virtual void undo() override;

        private:
            GMCanvas *scene_;
            QString meshOld_;
        };
    } //End namespace undoCommands
} //End namespace GMView

#endif // GUICHANGE_H
