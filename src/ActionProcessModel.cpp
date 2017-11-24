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

#include <ActionProcessModel.h>
#include <FaceModel.h>
using FaceTools::ActionProcessModel;
using FaceTools::FaceControl;


ActionProcessModel::ActionProcessModel( const std::string& dname, const std::string& fname, bool onMesh, bool onMeta, bool onView)
    : FaceTools::FaceAction(), _dname(dname.c_str()), _icon( fname.c_str()), _onMesh(onMesh), _onMeta(onMeta), _onView(onView)
{
    init();
    setEnabled(false);
}   // end ctor


void ActionProcessModel::setControlled( FaceControl* fcont, bool enable)
{
    assert( fcont);
    fcont->disconnect( this);
    _fconts.erase(fcont);
    if ( enable)
    {
        checkActionAdd( fcont);
        if ( _onMeta)
            connect( fcont, &FaceControl::metaUpdated, this, &ActionProcessModel::checkEnable);
        if ( _onMesh)
            connect( fcont, &FaceControl::meshUpdated, this, &ActionProcessModel::checkEnable);
        if ( _onView)
            connect( fcont, &FaceControl::viewUpdated, this, &ActionProcessModel::checkEnable);
    }   // end else
    setEnabled( !_fconts.empty());
}   // end setControlled


bool ActionProcessModel::doAction()
{
    boost::unordered_set<FaceModel*> processed;
    foreach ( FaceControl* fcont, _fconts)
    {
        FaceModel* fmodel = fcont->getModel();
        if ( processed.count( fmodel) == 0 || _onView)
        {
            assert( this->isActionable( fcont));
            if ( this->operator()( fcont))    // Do the action (implemented in child)
                processed.insert( fmodel);
        }   // end if
        else
            fcont->resetVisualisation();
    }   // end foreach
    return !processed.empty();
}   // end doAction


// private slot
void ActionProcessModel::checkEnable()
{
    checkActionAdd( qobject_cast<FaceControl*>(sender()));
    setEnabled( !_fconts.empty());
}   // end checkEnable


// private
void ActionProcessModel::checkActionAdd( FaceControl* fcont)
{
    assert(fcont);
    _fconts.erase(fcont);
    if ( this->isActionable( fcont))
        _fconts.insert(fcont);
}   // end checkActionAdd
