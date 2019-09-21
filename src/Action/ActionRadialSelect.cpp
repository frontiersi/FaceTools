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

#include <Action/ActionRadialSelect.h>
#include <Vis/FaceView.h>
#include <LndMrk/LandmarkSet.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::Event;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::FaceLateral;
using MS = FaceTools::Action::ModelSelector;


ActionRadialSelect::ActionRadialSelect( const QString& dn, const QIcon& ico)
    : ActionVisualise( dn, ico, _vis = new RadialSelectVisualisation)
{
    addPurgeEvent( Event::GEOMETRY_CHANGE);
}   // end ctor


ActionRadialSelect::~ActionRadialSelect()
{
    delete _vis;
}   // end dtor


double ActionRadialSelect::radius() const { return _handler ? _handler->radius() : 0.0;}

cv::Vec3f ActionRadialSelect::centre() const { return _handler ? _handler->centre() : cv::Vec3f(0,0,0);}


size_t ActionRadialSelect::selectedFaces( IntSet& fs) const { return _handler ? _handler->selectedFaces(fs) : 0;}


bool ActionRadialSelect::checkEnable( Event e)
{
    return MS::interactionMode() == IMode::CAMERA_INTERACTION && ActionVisualise::checkEnable(e);
}   // end checkEnable


void ActionRadialSelect::doAction( Event e)
{
    ActionVisualise::doAction( e);

    const FV* fv = MS::selectedView();
    const FM* fm = fv->data();

    if ( isChecked())
    {
        fm->lockForRead();
        cv::Vec3f cpos = fm->centreFront();
        const LandmarkSet& lmks = fm->currentAssessment()->landmarks();

        const QPoint& mpos = primedMousePos();
        // In the first case, select as the centre the point projected onto the surface my the
        // given 2D point (mouse coords). In the second instance, use pronasale if available,
        // otherwise just use the point on the surface closest to the model's centre.
        if ( !fv->projectToSurface( mpos, cpos) && lmks.hasCode( Landmark::PRN))
            cpos = lmks.pos( Landmark::PRN);
        else
            cpos = fm->findClosestSurfacePoint(cpos);

        // If landmarks set, get the radius as 2.3 times the distance between pronasale and pupils.
        double rad = radius();
        if ( lmks.hasCode( Landmark::P) && lmks.hasCode( Landmark::PRN))
        {
            cv::Vec3f mp = 0.5f * (lmks.pos(Landmark::P, FACE_LATERAL_LEFT) + lmks.pos(Landmark::P, FACE_LATERAL_RIGHT));
            rad = 2.3 * cv::norm( mp - lmks.pos(Landmark::PRN));
        }   // end if

        if ( !_handler || _handler->model() != fm)
        {
            _handler = std::shared_ptr<RadialSelectHandler>( new RadialSelectHandler( *_vis, fm));
            rad = radius();
        }   // end if
        fm->unlock();

        _handler->set( cpos, rad);   // Causes visualisation to be updated
        _vis->setHighlighted( fm, false);
    }   // end isChecked
}   // end doAction


void ActionRadialSelect::doAfterAction( Event e)
{
    ActionVisualise::doAfterAction( e);
    MS::clearStatus();
    if ( isChecked())
        MS::showStatus( "Reposition area by left-click and dragging the centre handle; change radius using the mouse wheel.");
}   // end doAfterAction


void ActionRadialSelect::purge( const FM* fm, Event e)
{
    _handler = nullptr;
    ActionVisualise::purge(fm, e);
}   // end purge
