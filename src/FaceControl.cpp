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

#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <FaceModelViewer.h>
#include <VtkTools.h>   // RVTK
using FaceTools::FaceModel;
using FaceTools::FaceControl;
using FaceTools::FaceModelViewer;
using FaceTools::Vis::FaceView;


// public
FaceControl::FaceControl( FaceModel* fm) : _fdata(fm), _fview(NULL)
{
    _fview = new FaceView(this);
}   // end ctor


// public
FaceControl::~FaceControl()
{
    delete _fview;
}   // end dtor


void FaceControl::transform( const cv::Matx44d& m)
{
    _fdata->transform(m);
    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK(m);
    _fview->transform( vm);
}   // end transform


void FaceControl::fixTransformFromView()
{
    const vtkMatrix4x4* m = _fview->transform(NULL);  // Transform according to user transform
    assert(m);
    cv::Matx44d cm = RVTK::toCV(m);
    _fdata->transform(cm);
}   // end fixTransformFromView


// public
void FaceControl::setViewer( FaceModelViewer* v) { _fview->setViewer(v);}
FaceModelViewer* FaceControl::viewer() const { return static_cast<FaceModelViewer*>(_fview->viewer());}


/*
// public
int FaceControl::updateLandmark( const std::string& lm, const cv::Vec3f* pos)
{
    LandmarkSet* lset = _fdata->landmarks();
    int id = -1;
    if ( pos != NULL)
        id = lset->set(lm, *pos);    // Set landmark with new position or add if not present
    else if ( lset->has(lm))
    {
        id = lset->get(lm)->id;    // Will be in invalid ID after deletion
        lset->erase(lm);
    }   // end else if
    _lview->refreshLandmark(id);
    return id;
}   // end updateLandmark


// public
bool FaceControl::updateLandmark( int id, const cv::Vec3f& v)
{
    LandmarkSet* lset = _fdata->landmarks();
    if ( !lset->has(id))
        return false;
    lset->set( id, v);
    _lview->refreshLandmark(id);
    return true;
}   // end updateLandmark


// public
bool FaceControl::changeLandmarkName( int id, const std::string& newName)
{
    bool changed = false;
    LandmarkSet* lset = _fdata->landmarks();
    if ( lset->has(id))
    {
        changed = lset->changeName( id, newName);
        _lview->refreshLandmark(id);
    }   // end if
    return changed;
}   // end changeLandmarkName


// public
bool FaceControl::changeLandmarkupdateLandmark( int id, bool visible, bool movable, bool deletable)
{
    bool changed = false;
    LandmarkSet* lset = _fdata->landmarks();
    if ( lset->has(id))
    {
        FaceTools::Landmarks::Landmark* lmk = lset->get(id);
        lmk->visible = visible;
        lmk->movable = movable;
        lmk->deletable = deletable;
        _lview->refreshLandmark(id);
    }   // end if
    return changed;
}   // end updateLandmark

*/
