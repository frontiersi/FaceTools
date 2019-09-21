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

#include <Vis/LandmarksVisualisation.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::FaceLateral;
using FaceTools::Action::Event;
using FaceTools::Landmark::LandmarkSet;

LandmarksVisualisation::~LandmarksVisualisation()
{
    while (!_views.empty())
        purge(const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


bool LandmarksVisualisation::isAvailable( const FM* fm) const
{
    assert(fm);
    return !fm->currentAssessment()->landmarks().empty();
}   // end isAvailable


void LandmarksVisualisation::apply( FV* fv, const QPoint*)
{
    assert(fv);
    if ( !_hasView(fv))
    {
        _views[fv] = new LandmarkSetView;
        syncLandmarks( fv->data());
    }   // end if
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


bool LandmarksVisualisation::purge( FV* fv, Event)
{
    if ( _hasView(fv))
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


void LandmarksVisualisation::setVisible( FV* fv, bool v)
{
    assert(fv);
    if ( _hasView(fv))
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool LandmarksVisualisation::isVisible( const FV *fv) const
{
    bool vis = false;
    if ( _hasView(fv))
        vis = !_views.at(fv)->visible().empty();
    return vis;
}   // end isVisible


void LandmarksVisualisation::setLandmarkVisible( const FM* fm, int lm, bool v)
{
    assert(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ if ( this->_hasView(fv)) _views.at(fv)->showLandmark(v,lm);});
}   // end setLandmarkVisible


void LandmarksVisualisation::setLandmarkHighlighted( const FM* fm, int lm, FaceLateral lat, bool v)
{
    assert(fm);
    if ( !fm)
        return;
    for ( FV* fv : fm->fvs())
    {
        if ( this->_hasView(fv))
            _views.at(fv)->highlightLandmark(v, lm, lat);
    }   // end for
}   // end setLandmarkHighlighted


void LandmarksVisualisation::setHighlighted( const FM* fm)
{
    for ( auto& p : _views) // Unhighlight all
        p.second->setColour( LandmarkSetView::BASE0_COL);

    if ( fm)    // Highlight just the landmarks of the given model.
    {
        for ( FV* fv : fm->fvs())
            if ( _hasView(fv))
                _views.at(fv)->setColour( LandmarkSetView::SPEC0_COL);
    }   // end if
}   // end setHighlighted


void LandmarksVisualisation::updateLandmark( const FM* fm, int id)
{
    assert(fm);
    if ( !fm)
        return;

    const LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    const bool isBilateral = LDMKS_MAN::landmark(id)->isBilateral();

    for ( FV* fv : fm->fvs())
    {
        if ( _hasView(fv))
        {
            const vtkMatrix4x4* T = fv->actor()->GetMatrix();
            LandmarkSetView* view = _views.at(fv);
            if ( isBilateral)
            {
                view->set( id, FACE_LATERAL_LEFT, lmks.pos(id, FACE_LATERAL_LEFT), T);
                view->set( id, FACE_LATERAL_RIGHT, lmks.pos(id, FACE_LATERAL_RIGHT), T);
            }   // end if
            else
                view->set( id, FACE_LATERAL_MEDIAL, lmks.pos(id, FACE_LATERAL_MEDIAL), T);
        }   // end if
    }   // end for
}   // end updateLandmark


void LandmarksVisualisation::syncLandmarks( const FM* fm)
{
    if ( !fm)
        return;

    const LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    if ( lmks.empty())
    {
        const FVS& fvs = fm->fvs();
        for ( FV* fv : fvs)
            purge(fv, Event::NONE);
    }   // end if
    else
    {
        for ( int id : lmks.ids())
            updateLandmark( fm, id);
    }   // end else
}   // end syncLandmarks


int LandmarksVisualisation::landmarkId( const FV* fv, const vtkProp* prop, FaceLateral &lat) const
{
    return _hasView(fv) ? _views.at(fv)->landmarkId( prop, lat) : -1;
}   // end landmarkId


bool LandmarksVisualisation::belongs( const vtkProp* p, const FV* fv) const
{
    bool b = false;
    if ( _hasView(fv))
    {
        FaceLateral ignored;
        b = _views.at(fv)->landmarkId(p, ignored) >= 0;
    }   // end if
    return b;
}   // end belongs


void LandmarksVisualisation::syncToViewTransform( const FV* fv, const vtkMatrix4x4* d)
{
    if ( _hasView(fv))
        _views.at(fv)->pokeTransform(d);
}   // end syncToViewTransform


bool LandmarksVisualisation::_hasView( const FV* fv) const { return _views.count(fv) > 0;}
