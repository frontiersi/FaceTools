/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_EDIT_PATHS_H
#define FACE_TOOLS_ACTION_EDIT_PATHS_H

/**
 * A visualisation with an internally constructed interactor to edit paths on FaceModels.
 */

#include "ActionVisualise.h"
#include <PathSetVisualisation.h>
#include <PathSetInteractor.h>
#include <QStatusBar>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionEditPaths : public ActionVisualise
{ Q_OBJECT
public:
    ActionEditPaths( const QString& dname, const QIcon& icon, FEEI*, QStatusBar* sb=nullptr);
    ~ActionEditPaths() override;

    Interactor::PathSetInteractor* interactor() override { return _interactor;}

protected slots:
    void doOnEditedPath( const FaceControl*);
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;

private:
    Vis::PathSetVisualisation *_vis;
    Interactor::PathSetInteractor *_interactor;
    QStatusBar *_sbar;
};  // end class

}   // end namespace
}   // end namespace

#endif
