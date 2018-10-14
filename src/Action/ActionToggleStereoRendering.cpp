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

#include <ActionToggleStereoRendering.h>
#include <FaceModelViewer.h>
#include <vtkRenderWindow.h>
using FaceTools::Action::ActionToggleStereoRendering;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMV;


ActionToggleStereoRendering::ActionToggleStereoRendering( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _sren(false)
{
    setCheckable( true, false);
}   // end ctor


namespace {

void setStereoRendering( FMV* v, bool enable)
{
    vtkRenderWindow* rwin = v->getRenderWindow();
    rwin->SetStereoRender(enable);
    v->updateRender();
}   // end setStereoRendering

}   // end namespace


void ActionToggleStereoRendering::addViewer( FMV* v)
{
    _viewers.insert(v);
    setStereoRendering( v, _sren);
}   // end addViewer


bool ActionToggleStereoRendering::testIfCheck( const FV*) const { return _sren;}


bool ActionToggleStereoRendering::doAction( FVS&, const QPoint&)
{
    _sren = isChecked();
    for ( FMV* v : _viewers)
        setStereoRendering( v, _sren);
    return true;
}   // end doAction
