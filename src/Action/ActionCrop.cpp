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
#include <ObjModelCopier.h>
using FaceTools::Action::ActionCrop;
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionVisualise;
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionCrop::ActionCrop( const QString& dn, const QIcon& ico, QStatusBar* sbar)
    : FaceAction(dn, ico, true/*disable before other*/), _sbar(sbar)
{
    _bvis = new BoundingVisualisation( dn, ico);
    _vact = new ActionVisualise( _bvis);
    _interactor = new RadialSelectInteractor( _bvis);
    connect( _interactor, &RadialSelectInteractor::onSetNewCentre, this, &ActionCrop::doOnSetNewCentre);
    connect( _interactor, &RadialSelectInteractor::onSetNewRadius, this, &ActionCrop::doOnSetNewRadius);
    addChangeTo( DATA_CHANGE);
    addRespondTo( VIEW_CHANGE); // The view change being responded to is the application of the associated BoundingVisualiation
}   // end ctor


ActionCrop::~ActionCrop()
{
    delete _interactor;
    delete _vact;
    delete _bvis;
}   // end dtor


// private slot
void ActionCrop::doOnSetNewCentre( FaceControl* fc, const cv::Vec3f& v)
{
    _bvis->setCentre( fc, v);
    fc->viewer()->updateRender();
}   // end doOnSetNewCentre


// private slot
void ActionCrop::doOnSetNewRadius( FaceControl* fc, double r)
{
    _bvis->setRadius( fc, r);
    fc->viewer()->updateRender();
}   // end doOnSetNewRadius


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
    FaceModel* fm = fc->data();

    double rad = _bvis->radius( fc);
    cv::Vec3f v = _bvis->centre( fc);
    int s = fm->kdtree().find(v);

    using namespace RFeatures;
    const ObjModel* model = fm->cmodel();
    ObjModelRegionSelector::Ptr cropper = ObjModelRegionSelector::create( model, v, s);
    cropper->setRadius( rad);
    IntSet cfids;
    cropper->getRegionFaces( cfids);
    assert( !cfids.empty());

    // Copy the subset of faces into a new model
    ObjModelCopier copier( model);
    std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.addTriangle(fid);});
    fm->updateData( copier.getCopiedModel());
    return true;
}   // end doAction
