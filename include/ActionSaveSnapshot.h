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

#ifndef FACE_TOOLS_ACTION_SAVE_SNAPSHOT_H
#define FACE_TOOLS_ACTION_SAVE_SNAPSHOT_H

#include <FaceActionInterface.h>    // FaceTools
#include <ModelInteractor.h>
#include <boost/unordered_set.hpp>

namespace FaceTools {

class FaceTools_EXPORT ActionSaveSnapshot : public FaceAction
{ Q_OBJECT
public:
    explicit ActionSaveSnapshot( const std::string& iconfilename);

    virtual const QIcon* getIcon() const { return &_icon;}
    virtual QString getDisplayName() const { return "Save Snapshot";}

    virtual void addInteractor( ModelInteractor*);
    virtual void removeInteractor( ModelInteractor*);

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    const QIcon _icon;
    boost::unordered_set<ModelInteractor*> _interactors;
};  // end class

}   // end namespace

#endif

