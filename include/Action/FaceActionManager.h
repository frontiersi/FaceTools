/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_FACE_ACTION_MANAGER_H
#define FACE_TOOLS_FACE_ACTION_MANAGER_H

#include "ModelSelector.h"
#include "FaceActionGroup.h"
#include "ActionExecutionQueue.h"
#include <ViewerInteractionManager.h>
#include <FaceModelManager.h>
#include <QMutex>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT FaceActionManager : public QObject
{ Q_OBJECT
public:
    FaceActionManager( FMV *defaultViewer, FileIO::FaceModelManager*);
    ~FaceActionManager() override;

    void addViewer( FMV* v) { _interactions->addViewer(v);}

    QAction* addAction( FaceAction*);       // Return added action's QAction if added okay (duplicates not allowed).

    ModelSelector* selector() { return &_selector;} // Get selector for programmatic selection of Vis::FV instances.

signals:
    void addedAction( FaceAction*); // Emitted whenever an action added
    void onUpdateSelected( FM*);

private slots:
    void doOnActionStarting();
    void doOnChangedData( Vis::FV*);
    void doOnActionFinished( EventSet, FVS, bool);

private:
    FileIO::FaceModelManager *_fmm;
    Interactor::ViewerInteractionManager *_interactions;
    ModelSelector _selector;
    std::unordered_set<FaceAction*> _actions;
    ActionExecutionQueue _aqueue;
    QMutex _mutex;

    void processFinishedAction( FaceAction*, EventSet&, FVS&);
    void setReady( Vis::FV*, bool);
    void close( FM*);

    FaceActionManager( const FaceActionManager&) = delete;
    void operator=( const FaceActionManager&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
