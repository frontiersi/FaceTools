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

#include <ActionComponentSelect.h>
#include <FaceTools.h>
#include <FaceModelViewer.h>
using FaceTools::Action::ActionComponentSelect;
using FaceTools::Action::ActionVisualise;
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::FV;


ActionComponentSelect::ActionComponentSelect( MEEI* meei)
    : ActionVisualise( _vis = new BoundingVisualisation)
{
    //connect( meei, &MEEI::onEnterComponent, [=](auto fv, int c){ _vis->setHighlighted( fv, c, true); fv->viewer()->updateRender();});
    //connect( meei, &MEEI::onLeaveComponent, [=](auto fv, int c){ _vis->setHighlighted( fv, c, false); fv->viewer()->updateRender();});
}   // end ctor


ActionComponentSelect::~ActionComponentSelect() { delete _vis;}
