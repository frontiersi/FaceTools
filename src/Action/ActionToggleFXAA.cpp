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

#include <ActionToggleFXAA.h>
#include <FaceModelViewer.h>
//#include <vtkFXAAOptions.h>
#include <vtkRenderer.h>
using FaceTools::Action::ActionToggleFXAA;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::FMV;


ActionToggleFXAA::ActionToggleFXAA( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
}   // end ctor


namespace {

void setAAEnabled( FMV* v, bool enable)
{
    vtkRenderer* ren = v->getRenderer();
    //vtkFXAAOptions* fxaa = ren->GetFXAAOptions();
    ren->SetUseFXAA( enable);
    v->updateRender();
}   // end setAAEnabledAA

}   // end namespace


void ActionToggleFXAA::addViewer( FMV* v)
{
    _viewers.insert(v);
    setAAEnabled( v, isChecked());
}   // end addViewer


bool ActionToggleFXAA::doAction( FVS&, const QPoint&)
{
    for ( FMV* v : _viewers)
        setAAEnabled( v, isChecked());
    return true;
}   // end doAction
