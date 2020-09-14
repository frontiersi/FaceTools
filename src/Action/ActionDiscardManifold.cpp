/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionDiscardManifold.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDiscardManifold;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Landmark::LandmarkSet;
using MS = FaceTools::Action::ModelSelector;


ActionDiscardManifold::ActionDiscardManifold( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _mid(-1)
{
    setAsync(true);
    addRefreshEvent( Event::MESH_CHANGE);
}   // end ctor


namespace  {
bool manifoldIdInRange( int mid)
{
    bool inRng = false;
    if ( MS::isViewSelected())
    {
        const FM* fm = MS::selectedModel();
        inRng = mid >= 0 && mid < static_cast<int>(fm->manifolds().count());
    }   // end if
    return inRng;
}   // end manifoldIdInRange
}   // end namespace


bool ActionDiscardManifold::isAllowed( Event)
{
    const FV *fv = MS::selectedView();
    return fv && MS::interactionMode() == IMode::CAMERA_INTERACTION
              && (fv->data()->manifolds().count() > 1)
              && (manifoldIdInRange(_mid) || fv->isPointOnFace( primedMousePos()));
}   // end isAllowed


bool ActionDiscardManifold::doBeforeAction( Event)
{
    const FV* fv = MS::selectedView();
    const FM* fm = fv->data();
    fm->lockForRead();

    Vec3f cpos;
    if ( fv->projectToSurface( primedMousePos(), cpos))
    {
        const int svidx = fm->findVertex( cpos);
        const int fx = *fm->mesh().faces(svidx).begin(); // Get an attached face 
        _mid = fm->manifolds().fromFaceId(fx); // Manifold ID
        assert( _mid >= 0);
    }   // end if

    const size_t numFaceRemove = manifoldIdInRange(_mid) ? fm->manifolds()[_mid].faces().size() : 0;
    fm->unlock();

    bool goOk = false;
    if ( numFaceRemove > 0)
    {
        const int rv = QMessageBox::question( static_cast<QWidget*>(parent()),
                              tr("Discard Manifold"),
                              QString("%1 triangles from manifold %2 will be removed. Continue?").arg(numFaceRemove).arg(1+_mid),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        goOk = rv == QMessageBox::Yes;
    }   // end if

    if ( goOk)
        MS::showStatus("Removing manifold...");
    return goOk;
}   // end doBeforeAction


void ActionDiscardManifold::doAction( Event)
{
    storeUndo(this, Event::MESH_CHANGE | Event::LANDMARKS_CHANGE);

    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    r3d::Mesh::Ptr mobj = fm->mesh().deepCopy();
    const IntSet& polys = fm->manifolds()[_mid].faces();  // Faces to discard
    for ( int f : polys)
        mobj->removeFace(f);

    _mid = -1;

    // FM::update removes any unused vertices resulting from the removal of faces
    // and then ensures that the vertices are in sequential order.
    fm->update( mobj, true, true);
    fm->unlock();
}   // end doAction


Event ActionDiscardManifold::doAfterAction( Event)
{
    MS::showStatus( "Finished removing manifold.", 5000);
    return Event::MESH_CHANGE | Event::LANDMARKS_CHANGE;
}   // end doAfterAction
