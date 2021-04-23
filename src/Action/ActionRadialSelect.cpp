/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <Interactor/RadialSelectHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceModel.h>
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionRadialSelect::ActionRadialSelect( const QString& dn, const QIcon& ico)
    : ActionVisualise( dn, ico, &MS::handler<RadialSelectHandler>()->visualisation())
{
    addPurgeEvent( Event::MESH_CHANGE);
    addRefreshEvent( Event::MESH_CHANGE);
}   // end ctor


bool ActionRadialSelect::update( Event e)
{
    const bool v = ActionVisualise::update(e);
    RadialSelectHandler *handler = MS::handler<RadialSelectHandler>();
    assert( handler);
    handler->refresh();
    return v;
}   // end update


void ActionRadialSelect::doAction( Event e)
{
    RadialSelectHandler *handler = MS::handler<RadialSelectHandler>();
    assert( handler);

    const FV* fv = MS::selectedView();
    const FM* fm = fv->data();

    if ( isChecked())
    {
        fm->lockForRead();
        // In the first case, select as the centre the point projected onto the surface my the
        // given 2D point (mouse coords). In the second instance, use pronasale if available,
        // otherwise just use the point on the surface closest to the model's centre.
        Vec3f tpos = fm->findClosestSurfacePoint( fm->centreFront());
        const Landmark::LandmarkSet& lmks = fm->currentLandmarks();
        const QPoint& mpos = primedMousePos();
        if ( !fv->projectToSurface( mpos, tpos) && lmks.has( LMAN::codeId(Landmark::PRN)))
            tpos = lmks.pos( Landmark::PRN);

        float rad = handler->radius();
        if ( fm->hasLandmarks())
            rad = sqrtf(fm->currentLandmarks().sqRadius());
        else if ( rad == 0.0f)
            rad = fm->bounds()[0]->diagonal()/4;

        fm->unlock();
        handler->init( fm, tpos, rad);
    }   // end isChecked
    else
        handler->reset( fm);

    ActionVisualise::doAction( e);
}   // end doAction


Event ActionRadialSelect::doAfterAction( Event e)
{
    MS::clearStatus();
    if ( isChecked())
        MS::showStatus( "Reposition by left-click and dragging the centre handle; change radius using the mouse wheel.");
    return ActionVisualise::doAfterAction( e);
}   // end doAfterAction


void ActionRadialSelect::purge( const FM *fm)
{
    RadialSelectHandler *handler = MS::handler<RadialSelectHandler>();
    assert( handler);
    handler->reset( fm);
}   // end purge
