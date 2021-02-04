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

#include <Action/ActionToggleStereo.h>
#include <FaceModelViewer.h>
#include <vtkRenderWindow.h>
using FaceTools::Action::ActionToggleStereo;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;


namespace {
void setStereo( FMV* v, bool enable)
{
    vtkRenderWindow* rwin = v->getRenderWindow();
    rwin->SetStereoRender(enable);
}   // end setStereo
}   // end namespace


ActionToggleStereo::ActionToggleStereo( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
    doAction( Event::NONE);
}   // end ctor


void ActionToggleStereo::doAction( Event)
{
    for ( FMV* v : ModelSelect::viewers())
        setStereo( v, isChecked());
}   // end doAction
