/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include "ActionVisualise.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionEditPaths : public ActionVisualise
{ Q_OBJECT
public:
    ActionEditPaths( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Display and edit user measurements.";}

protected:
    Event doAfterAction( Event) override;

private slots:
    void _doOnStartedDrag( int pid, int hid);
    void _doOnFinishedDrag( int pid, int hid);
    void _doOnEnterHandle( int pid, int hid);
    void _doOnLeaveHandle( int pid, int hid);

private:
    FVS _tchanged;
    void _setTempTransparency(bool);
    void _changeTransparency(bool, Vis::FV*);
};  // end class

}}   // end namespaces

#endif
