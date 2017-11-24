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

#ifndef FACE_TOOLS_ACTION_SET_FOCUS_H
#define FACE_TOOLS_ACTION_SET_FOCUS_H

#include "FaceActionInterface.h"
#include "ModelViewer.h"
#include "FaceControl.h"

namespace FaceTools {

class FaceTools_EXPORT ActionSetFocus : public FaceAction
{ Q_OBJECT
public:
    ActionSetFocus();

    virtual QString getDisplayName() const { return "Set Camera Focus";}

    virtual void addController( FaceControl*);
    virtual void removeController( FaceControl*);

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    FaceControl *_fcont;
};  // end class

}   // end namespace

#endif

