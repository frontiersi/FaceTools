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

#include <ActionCrop.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <FaceModelViewer.h>
#include <ObjModelRegionSelector.h>
using FaceTools::Action::ActionCrop;
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionVisualise;
using FaceTools::Vis::BoundaryVisualisation;
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionCrop::ActionCrop( QStatusBar* sbar)
    : FaceAction(true/*disable before other*/), _icon( ":/icons/CROP_FACE"), _sbar(sbar)
{
    _bvis = new BoundaryVisualisation( "Cropping Mode", *getIcon());
    _vact = new ActionVisualise( _bvis);
    _interactor = new RadialSelectInteractor( _bvis);
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( VISUALISATION_CHANGED);
}   // end ctor


ActionCrop::~ActionCrop()
{
    delete _interactor;
    delete _vact;
    delete _bvis;
}   // end dtor


bool ActionCrop::testReady( FaceControl* fc)
{
    return _bvis->isApplied(fc);
}   // end testReady


void ActionCrop::tellReady( FaceControl* fc, bool isReady)
{
    const static QString smsg( tr("Reposition with left drag; change size with right drag."));
    // Show or hide status info
    if ( isReady)   // ready if _bvis->isApplied(fc)
    {
        _interactor->setViewer( fc->viewer());
        _sbar->showMessage(smsg, 10000);    // 10 sec temp
    }   // end if
    else
    {
        _interactor->setViewer(NULL);
        if ( _sbar->currentMessage() == smsg)
            _sbar->clearMessage();
    }   // end else
}   // end tellReady


bool ActionCrop::doAction( FaceControlSet& rset)
{
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();
    double rad = _bvis->radius( fc);
    cv::Vec3f v = _bvis->centre( fc);
    FaceModel* fm = fc->data();
    int s = fm->kdtree()->find(v);
    RFeatures::ObjModel::Ptr cmodel = FaceTools::crop( fm->model(), v, s, rad); // Single connected component
    fm->setModel(cmodel);
    return true;
}   // end doAction
