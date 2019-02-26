/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ModelSelector.h>
#include <BoundingVisualisation.h>
#include <FaceActionManager.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <FaceModel.h>
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceActionManager;
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FM;

ModelSelector::Ptr ModelSelector::_me;


void ModelSelector::create( FaceActionManager* fam, FMV* viewer)
{
    assert(!_me);
    _me = Ptr( new ModelSelector( fam, viewer), [](ModelSelector* d) { delete d;});
}   // end me


// private
ModelSelector::ModelSelector( FaceActionManager* fam, FMV *viewer)
{
    _msi.setViewer(viewer);
    QObject::connect( &_msi, &ModelSelectInteractor::onSelected, fam, &FaceActionManager::doOnSelected);
}   // end ctor


FMV* ModelSelector::viewer() { return static_cast<FMV*>(_me->_msi.viewer());}

FV* ModelSelector::addFaceView( FM* fm, FMV* tv)
{
    if ( !_me)
    {
        std::cerr << "[ERROR] FaceTools::Action::ModelSelector::addFaceView: ModelSelector instance not yet created!" << std::endl;
        return nullptr;
    }   // end if

    if ( !tv)
        tv = static_cast<FMV*>(_me->_msi.viewer());
    FV* fv = new FV( fm, tv); // Attaches the viewer and creates the base models (calls FaceView::reset)
    _me->_msi.add(fv);    // Does NOT cause onSelected(fv, true) to fire
    return fv;
}   // end addFaceView


void ModelSelector::removeFaceView( FV* fv)
{
    if ( !_me)
    {
        std::cerr << "[ERROR] FaceTools::Action::ModelSelector::addFaceView: ModelSelector instance not yet created!" << std::endl;
        return;
    }   // end if

    _me->_msi.remove(fv);    // Called *before* the viewer is detached from the FaceView.
    FMV* vwr = fv->viewer();
    delete fv;
    vwr->updateRender(); // Extra render needed after detaching the viewer.
    std::cerr << "[INFO] FaceTools::Action::ModelSelector::removeFaceView: " << vwr->attached().size() << " attached" << std::endl;
}   // end removeFaceView


void ModelSelector::remove( FM* fm)
{
    while ( !fm->fvs().empty())
        removeFaceView( fm->fvs().first());
}   // end remove


void ModelSelector::setSelected( FV* fv, bool enable)
{
    if ( !_me)
    {
        std::cerr << "[ERROR] FaceTools::Action::ModelSelector::addFaceView: ModelSelector instance not yet created!" << std::endl;
        return;
    }   // end if

    if ( _me->_msi.isSelected(fv) != enable)    // Do nothing if no change in selection
        _me->_msi.setSelected( fv, enable); // Causes onSelected to fire
}   // end setSelected


void ModelSelector::setSelectEnabled( bool v)
{
    _me->_msi.setEnabled(v);
}   // end setSelectedEnabled
