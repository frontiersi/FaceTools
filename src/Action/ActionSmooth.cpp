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

#include <Action/ActionSmooth.h>
#include <FaceModelCurvatureStore.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <r3d/Smoother.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionSmooth;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;

// static definitions
double ActionSmooth::s_maxc(0.9);
size_t ActionSmooth::s_maxi(10);

void ActionSmooth::setMaxCurvature( double c) { s_maxc = std::max( 0.0, std::min( c, 1.0));}
void ActionSmooth::setMaxIterations( size_t i) { s_maxi = i;}


ActionSmooth::ActionSmooth( const QString& dn, const QIcon& ico) : FaceAction(dn, ico)
{
    addRefreshEvent( Event::SURFACE_DATA_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionSmooth::isAllowed( Event)
{
    return MS::isViewSelected() && FaceModelCurvatureStore::rvals( *MS::selectedModel());
}   // end isAllowed


void ActionSmooth::saveState( UndoState &us) const
{
    us.model()->lockForRead();
    us.setUserData( "Mesh", QVariant::fromValue( us.model()->mesh().deepCopy()));
    us.setUserData( "Ass", QVariant::fromValue( us.model()->currentAssessment()->deepCopy()));
    us.model()->unlock();
}   // end saveState


void ActionSmooth::restoreState( const UndoState &us)
{
    us.model()->lockForWrite();
    us.model()->update( us.userData("Mesh").value<r3d::Mesh::Ptr>(), false, false);
    us.model()->setAssessment( us.userData("Ass").value<FaceAssessment::Ptr>());
    us.model()->unlock();
}   // end restoreState


bool ActionSmooth::doBeforeAction( Event)
{
    _ev = Event::MESH_CHANGE;
    bool doit = true;
    if ( MS::selectedModelScopedRead()->hasLandmarks())
    {
        _ev |= Event::LANDMARKS_CHANGE;
        static const QString msg = tr("Smoothing will perturb landmarks positions. Are you sure you want to continue?");
        doit = QMB::Yes == QMB::warning( static_cast<QWidget*>(parent()), tr("Landmarks Present!"),
                                QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No);
    }   // end if

    if ( doit)
    {
        MS::showStatus( "Smoothing model surface...");
        storeUndo( this, _ev, false);   // Must provide custom UndoState
    }   // end goSmooth

    return doit;
}   // end doBeforeAction


void ActionSmooth::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    FaceModelCurvatureStore::WPtr cmap = FaceModelCurvatureStore::wvals( *fm);
    // Updates curvature data for the mesh but should be reconstructed anyway
    // so no need to call updateArrays.
    r3d::Smoother( maxCurvature(), maxIterations())( *mesh, cmap->vals());
    fm->update( mesh, false, true);
}   // end doAction


Event ActionSmooth::doAfterAction( Event)
{
    MS::showStatus("Finished smooth.", 5000);
    return _ev;
}   // end doAfterAction
