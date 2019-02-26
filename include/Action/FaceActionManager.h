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

#include "FaceActionGroup.h"
#include "ActionExecutionQueue.h"
#include <BoundingVisualisation.h>
#include <ViewerInteractionManager.h>
#include <QMutex>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceActionManager : public QObject
{ Q_OBJECT
public:
    explicit FaceActionManager( FMV *defaultViewer);
    ~FaceActionManager() override;

    void addViewer( FMV*);
    QAction* addAction( FaceAction*);       // Return added action's QAction if added okay (duplicates not allowed).

    void close( FM*);

signals:
    void addedAction( FaceAction*);
    void onUpdateSelected( FM*, bool);

public slots:
    void doOnSelected( Vis::FV*, bool);

private slots:
    void doOnActionStarting();
    void doOnChangedData( Vis::FV*);
    void doOnActionFinished( EventSet, FVS, bool);

private:
    Interactor::ViewerInteractionManager *_interactions;
    std::unordered_set<FaceAction*> _actions;
    ActionExecutionQueue _aqueue;
    QMutex _mutex;
    Vis::BoundingVisualisation _bvis;

    void processFinishedAction( FaceAction*, EventSet&, FVS&);

    FaceActionManager( const FaceActionManager&) = delete;
    void operator=( const FaceActionManager&) = delete;
};  // end class

}}   // end namespace

#endif
