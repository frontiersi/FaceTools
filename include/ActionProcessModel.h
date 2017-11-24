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

#ifndef FACE_TOOLS_ACTION_PROCESS_MODEL_H
#define FACE_TOOLS_ACTION_PROCESS_MODEL_H

#include "FaceActionInterface.h"
#include "FaceControl.h"

namespace FaceTools {

class FaceTools_EXPORT ActionProcessModel : public FaceAction
{ Q_OBJECT
public:
    ActionProcessModel( const std::string& dname, const std::string& iconfilename="",
                        bool onMeshUpdated=true,   // Action checked for availability on mesh update (true)
                        bool onMetaUpdated=true,   // Action checked for availability on meta update (true)
                        bool onViewUpdated=false); // Action checked for availability on view update (false)

    virtual const QIcon* getIcon() const { return &_icon;}
    virtual QString getDisplayName() const { return _dname;}

    virtual void setControlled( FaceControl*, bool);

    // Return true on success. This is called in such a way to ensure
    // that only a single FaceControl is called for a model - even if
    // multiple FaceControls for a single FaceModel exist. However, if
    // selected to update on view changes, this will be called for
    // EVERY FaceControl set as being under the control of this action.
    virtual bool operator()( FaceControl*) = 0;

    // Reimplement if FaceControl needs to be in a specific state
    // before the action can be applied to it.
    virtual bool isActionable( FaceControl*) const { return true;}

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    const QIcon _icon;
    const QString _dname;
    bool _onMesh, _onMeta, _onView;
    boost::unordered_set<FaceControl*> _fconts;
    void checkActionAdd( FaceControl*);
};  // end class

}   // end namespace

#endif


