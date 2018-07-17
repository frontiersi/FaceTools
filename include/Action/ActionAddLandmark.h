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

#ifndef FACE_TOOLS_ACTION_ADD_LANDMARK_H
#define FACE_TOOLS_ACTION_ADD_LANDMARK_H

#include "ActionEditLandmarks.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionAddLandmark : public FaceAction
{ Q_OBJECT
public:
    ActionAddLandmark( const QString& dname, const QIcon& icon, QWidget* parent=nullptr);

    void setEditor( ActionEditLandmarks *e) { _editor = e;}

protected slots:
    bool testEnabled() const override;
    bool doAction( FaceControlSet&) override;
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool);

private:
    ActionEditLandmarks *_editor;
    QWidget *_parent;
};  // end class

}   // end namespace
}   // end namespace

#endif
