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

#ifndef FACE_TOOLS_ACTION_SAVE_MODEL_AS_H
#define FACE_TOOLS_ACTION_SAVE_MODEL_AS_H

#include "FaceActionInterface.h"
#include "ModelInteractor.h"

namespace FaceTools {

class ActionSaveModelAs : public FaceAction
{ Q_OBJECT
public:
    explicit ActionSaveModelAs( const std::string& iconfilename);

    virtual const QIcon* getIcon() const { return &_icon;}
    virtual const QKeySequence* getShortcut() const { return &_keyseq;}
    virtual QString getDisplayName() const { return _dname;}

    virtual void setInteractive( ModelInteractor*, bool);

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    const QIcon _icon;
    const QString _dname;
    const QKeySequence _keyseq;
    FaceModel* _fmodel;
};  // end class

}   // end namespace

#endif


