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

#include <Action/ActionCrop.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <ObjModelCopier.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCrop;
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionCrop::ActionCrop( const QString& dn, const QIcon& ico, ActionRadialSelect *rs)
    : FaceAction(dn, ico), _rsel(rs)
{
    setAsync(true);
}   // end ctor


bool ActionCrop::checkEnable( Event)
{
    assert(_rsel);
    return MS::selectedView() && _rsel->isChecked();
}   // end testEnabled


bool ActionCrop::doBeforeAction( Event)
{
    MS::showStatus("Cropping model to selected region...");
    return true;
}   // end doBeforeAction


void ActionCrop::doAction( Event)
{
    storeUndo( this, {Event::GEOMETRY_CHANGE, Event::LANDMARKS_CHANGE, Event::CONNECTIVITY_CHANGE});
    IntSet cfids;
    _rsel->selectedFaces( cfids);

    // Copy the subset of faces into a new model
    if ( !cfids.empty())
    {
        using namespace RFeatures;
        FM* fm = MS::selectedModel();
        fm->lockForWrite();
        ObjModelCopier copier( fm->model());
        std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.add(fid);});
        ObjModel::Ptr nmod = copier.copiedModel();
        fm->update( nmod);
        fm->moveLandmarksToSurface();
        fm->unlock();
    }   // end if
}   // end doAction


void ActionCrop::doAfterAction( Event)
{
    MS::showStatus("Finished crop.", 5000);
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::LANDMARKS_CHANGE, Event::CONNECTIVITY_CHANGE});
}   // end doAfterAction
