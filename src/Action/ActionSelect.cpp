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

#include <Action/ActionSelect.h>
#include <Action/ActionOrientCamera.h>
#include <Interactor/LandmarksHandler.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
using FaceTools::Action::ActionSelect;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;

bool ActionSelect::s_showBoxes(true);


ActionSelect::ActionSelect() : FaceAction("ActionSelect"), _sfv(nullptr)
{
    addTriggerEvent( Event::MODEL_SELECT | Event::LOADED_MODEL | Event::MASK_CHANGE | Event::AFFINE_CHANGE);
}   // end ctor


bool ActionSelect::doBeforeAction( Event) { return MS::isViewSelected();}


void ActionSelect::doAction( Event e)
{
    FV *fv = MS::selectedView();
    Interactor::LandmarksHandler *h = MS::handler<Interactor::LandmarksHandler>();
    Vis::LandmarksVisualisation *lvis = h ? &h->visualisation() : nullptr;

    if ( _sfv && _sfv != fv)
    {
        _bvis.setVisible( _sfv, false);
        if ( lvis)
        {
            for ( const FV *sfv : _sfv->data()->fvs())
                lvis->setSelectedColour( sfv, false);
        }   // end if
        _sfv = nullptr;
    }   // end if

    _sfv = fv;
    if ( lvis)
    {
        for ( const FV *sfv : _sfv->data()->fvs())
            lvis->setSelectedColour( sfv, true);
    }   // end if

    if ( s_showBoxes)
        _sfv->apply( &_bvis);
    else
        _sfv->purge( &_bvis);
}   // end doAction


void ActionSelect::purge( const FM *fm)
{
    for ( FV *fv : fm->fvs())
    {
        if ( fv == _sfv)
        {
            fv->purge( &_bvis);
            _sfv = nullptr;
        }   // end if
    }   // end for
}   // end purge


// static
void ActionSelect::setShowBoundingBoxes( bool v) { s_showBoxes = v;}

