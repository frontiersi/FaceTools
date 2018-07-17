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

#include <ModelSelector.h>
#include <BoundingVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
using FaceTools::Action::ModelSelector;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::FaceModelViewer;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

// private
ModelSelector::ModelSelector( FaceModelViewer *viewer)
    : _interactor( new ModelSelectInteractor(true)) // Exclusive select
{
    _interactor->setViewer(viewer);
    connect( _interactor, &ModelSelectInteractor::onSelected, [this](FaceControl* fc, bool v){ emit onSelected( fc,v);});
}   // end ctor


// private
ModelSelector::~ModelSelector()
{
    FaceModelSet fms = _interactor->available().models();  // Copy out
    std::for_each( std::begin(fms), std::end(fms), [this](auto fm){ this->remove(fm);});
    delete _interactor;
}   // end dtor


// public
FaceControl* ModelSelector::addFaceControl( FaceModel* fm, FaceModelViewer* tv)
{
    if ( !tv)
        tv = static_cast<FaceModelViewer*>(_interactor->viewer());
    FaceControl* fc = new FaceControl( fm, tv); // Attaches the viewer and creates the base models (calls FaceView::reset)
    _interactor->add(fc);   // Will cause onSelected(fc, true) to fire
    return fc;
}   // end addFaceControl


// public
void ModelSelector::removeFaceControl( FaceControl* fc)
{
    _interactor->remove(fc);    // Called *before* the viewer is detached from the FaceControl.
    FaceModelViewer* viewer = fc->viewer();
    delete fc;
    viewer->updateRender(); // Extra render needed after detaching the viewer.
}   // end removeFaceControl


// public
void ModelSelector::remove( FaceModel* fm)
{
    while ( !fm->faceControls().empty())
        removeFaceControl( fm->faceControls().first());
}   // end remove


// public
void ModelSelector::select( FaceControl* fc, bool enable)
{
    if ( _interactor->isSelected(fc) != enable)    // Do nothing if no change in selection
        _interactor->setSelected( fc, enable);
}   // end select
