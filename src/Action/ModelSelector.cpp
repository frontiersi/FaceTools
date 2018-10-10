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
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <FaceModel.h>
using FaceTools::Action::ModelSelector;
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::FMV;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMS;
using FaceTools::FM;

// private
ModelSelector::ModelSelector( FMV *viewer)
{
    _interactor.setViewer(viewer);
    connect( &_interactor, &ModelSelectInteractor::onSelected, this, &ModelSelector::onSelected);
}   // end ctor


// private
ModelSelector::~ModelSelector()
{
    FMS fms = _interactor.available().models();  // Copy out
    std::for_each( std::begin(fms), std::end(fms), [this](FM* fm){ this->remove(fm);});
}   // end dtor


// public
FV* ModelSelector::addFaceView( FM* fm, FMV* tv)
{
    if ( !tv)
        tv = static_cast<FMV*>(_interactor.viewer());
    FV* fv = new FV( fm, tv); // Attaches the viewer and creates the base models (calls FaceView::reset)
    _interactor.add(fv);    // Does NOT cause onSelected(fv, true) to fire
    return fv;
}   // end addFaceView


// public
void ModelSelector::removeFaceView( FV* fv)
{
    _interactor.remove(fv);    // Called *before* the viewer is detached from the FaceView.
    FMV* vwr = fv->viewer();
    delete fv;
    vwr->updateRender(); // Extra render needed after detaching the viewer.
    std::cerr << "[INFO] FaceTools::Action::ModelSelector::removeFaceView: " << vwr->attached().size() << " attached" << std::endl;
}   // end removeFaceView


// public
void ModelSelector::remove( FM* fm)
{
    while ( !fm->fvs().empty())
        removeFaceView( fm->fvs().first());
}   // end remove


// public
void ModelSelector::setSelected( FV* fv, bool enable)
{
    if ( _interactor.isSelected(fv) != enable)    // Do nothing if no change in selection
        _interactor.setSelected( fv, enable);
}   // end setSelected


// private
void ModelSelector::doSwitchSelectedToViewer( ModelViewer* vwr)
{
    FV* fv = _interactor.selected();
    if ( !fv)
        return;

    for ( FV* f : fv->data()->fvs())
    {
        if ( f != fv && f->viewer() == vwr)
        {
            setSelected( fv, false);
            setSelected( f, true);
            break;
        }   // end if
    }   // end for
}   // end doSwitchSelectedToViewer
