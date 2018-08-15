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

#ifndef FACE_TOOLS_ACTION_RENAME_PATH_H
#define FACE_TOOLS_ACTION_RENAME_PATH_H

#include "ActionEditPaths.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionRenamePath : public FaceAction
{ Q_OBJECT
public:
    ActionRenamePath( const QString& dname, const QIcon&, ActionEditPaths*, QWidget *parent=nullptr);

private slots:
    bool testEnabled( const QPoint* mc=nullptr) const override;
    bool doAction( FaceControlSet&, const QPoint&) override;
    void doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool) override { cs.insert(METRICS_CHANGE);}

private:
    ActionEditPaths *_editor;
    QWidget *_parent;
};  // end class

}   // end namespace
}   // end namespace

#endif
