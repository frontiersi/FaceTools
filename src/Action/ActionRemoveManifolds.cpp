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

#include <ActionRemoveManifolds.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionRemoveManifolds;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Landmark::LandmarkSet;
using MS = FaceTools::Action::ModelSelector;


ActionRemoveManifolds::ActionRemoveManifolds( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _mid(-1)
{
    setAsync(true);
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


bool ActionRemoveManifolds::checkEnable( Event)
{
    bool enabled = false;
    if ( MS::isViewSelected() && MS::interactionMode() == IMode::CAMERA_INTERACTION)
    {
        // Ready if more than one manifold and the point is on the face.
        const FV* fv = MS::selectedView();
        const FM* fm = fv->data();
        fm->lockForRead();
        enabled = (fm->manifolds().count() > 1) && (manifoldIdInRange(_mid) || fv->isPointOnFace( primedMousePos()));
        fm->unlock();
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionRemoveManifolds::doBeforeAction( Event)
{
    const FV* fv = MS::selectedView();
    const FM* fm = fv->data();
    fm->lockForRead();

    cv::Vec3f cpos;
    if ( fv->projectToSurface( primedMousePos(), cpos))
    {
        const int svidx = fm->findVertex( cpos);
        const int fx = *fm->model().faces(svidx).begin(); // Get an attached polygon
        _mid = fm->manifolds().manifoldId(fx); // Manifold ID
        assert( _mid >= 0);
    }   // end if

    const size_t numManRemove = fm->manifolds().count() - 1;
    const size_t numFaceRemove = manifoldIdInRange(_mid) ? size_t(fm->model().numPolys()) - fm->manifolds().manifold(_mid)->polygons().size() : 0;
    fm->unlock();

    if ( numFaceRemove > 0 && requireConfirm())
    {
        const int rv = QMessageBox::question( qobject_cast<QWidget*>(parent()),
                              tr("Removing Other Manifolds"),
                              QString("%1 polygons from %2 manifolds will be removed. Continue?").arg(numFaceRemove).arg(numManRemove),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if ( rv == QMessageBox::No)
            _mid = -1;
    }   // end if

    if ( _mid >= 0)
        MS::showStatus("Removing manifolds from selected model...");
    return _mid >= 0;
}   // end doBeforeAction


void ActionRemoveManifolds::doAction( Event)
{
    storeUndo(this, {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});

    FM* fm = MS::selectedModel();
    fm->lockForWrite();

    RFeatures::ObjModelCopier copier( fm->model());
    const IntSet& fids = fm->manifolds().manifold(_mid)->polygons();
    for ( int fid : fids)
        copier.add(fid);
    RFeatures::ObjModel::Ptr mobj = copier.copiedModel();

    _mid = -1;
    fm->update( mobj);
    fm->moveLandmarksToSurface();
    fm->unlock();
}   // end doAction


void ActionRemoveManifolds::doAfterAction( Event)
{
    MS::showStatus("Finished removing manifolds.", 5000);
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});
}   // end doAfterAction

