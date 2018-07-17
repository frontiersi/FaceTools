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

#include <ActionToggleAxes.h>
#include <vtkProperty.h>
#include <algorithm>
using FaceTools::Action::ActionToggleAxes;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;


ActionToggleAxes::ActionToggleAxes( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
}   // end ctor


void ActionToggleAxes::addViewer( FaceModelViewer* v)
{
    vtkSmartPointer<vtkCubeAxesActor> actor = vtkSmartPointer<vtkCubeAxesActor>::New();
    actor->SetBounds( -200, 200, -200, 200, -200, 200);
    actor->DrawXGridlinesOn();
    actor->DrawYGridlinesOn();
    actor->DrawZGridlinesOn();

    actor->SetGridLineLocation( vtkCubeAxesActor::VTK_GRID_LINES_FURTHEST);
    //actor->SetFlyModeToStaticTriad();
    //actor->SetFlyModeToClosestTriad();
    actor->SetFlyModeToFurthestTriad();
    //actor->SetStickyAxes(true);
    actor->SetCamera( const_cast<vtkRenderer*>(v->getRenderer())->GetActiveCamera());

    actor->GetXAxesGridlinesProperty()->SetColor(0.1,0.4,0.1);
    actor->GetYAxesGridlinesProperty()->SetColor(0.1,0.4,0.1);
    actor->GetZAxesGridlinesProperty()->SetColor(0.1,0.4,0.1);

    actor->GetXAxesInnerGridlinesProperty()->SetColor(0.1,0.4,0.1);
    actor->GetYAxesInnerGridlinesProperty()->SetColor(0.1,0.4,0.1);
    actor->GetZAxesInnerGridlinesProperty()->SetColor(0.1,0.4,0.1);

    actor->GetXAxesGridlinesProperty()->SetLineWidth(1.0);
    actor->GetYAxesGridlinesProperty()->SetLineWidth(1.0);
    actor->GetZAxesGridlinesProperty()->SetLineWidth(1.0);

    _viewers[v] = actor;
}   // end addViewer


bool ActionToggleAxes::doAction( FaceControlSet&)
{
    for ( auto p : _viewers)
    {
        if ( isChecked())
            p.first->add(p.second);
        else
            p.first->remove(p.second);
        p.first->updateRender();
    }   // end for
    return true;
}   // end doAction
