/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_EDIT_LANDMARKS_H
#define FACE_TOOLS_ACTION_EDIT_LANDMARKS_H

#include "ActionVisualise.h"
#include <Vis/LandmarksVisualisation.h>
#include <Interactor/LandmarksHandler.h>
#include <QStatusBar>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionEditLandmarks : public ActionVisualise
{ Q_OBJECT
public:
    ActionEditLandmarks( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());
    ~ActionEditLandmarks() override;

    QString toolTip() const override { return "Showing facial landmarks if present.";}

protected:
    bool checkState( Event) override;
    bool doBeforeAction( Event) override;
    void doAfterAction( Event) override;

    UndoState::Ptr makeUndoState() const override;
    void restoreState( const UndoState*) override;

private:
    Vis::LandmarksVisualisation *_vis;
    std::shared_ptr<Interactor::LandmarksHandler> _handler;
};  // end class

}}   // end namespace

#endif
