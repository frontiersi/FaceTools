/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionUpdateThumbnail.h>
#include <Action/ActionOrientCamera.h>
#include <FaceModelCurvatureStore.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <r3dvis/OffscreenMeshViewer.h>
#include <QTools/QImageTools.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;

/*
// static definitions
std::unordered_map<const FM*, cv::Mat_<cv::Vec3b> > ActionUpdateThumbnail::_thumbs;
QReadWriteLock ActionUpdateThumbnail::_rwlock;
*/


ActionUpdateThumbnail::ActionUpdateThumbnail( int w, int h) : FaceAction("Thumbnail Updater"), _vsz( w,h)
{
    addTriggerEvent( Event::MESH_CHANGE | Event::LOADED_MODEL);
    setAsync(true);
}   // end ctor

/*
// static
cv::Mat ActionUpdateThumbnail::thumbnail( const FM *fm)
{
    if ( !fm)
        fm = MS::selectedModel();
    cv::Mat img;
    if ( _rwlock.tryLockForRead())
    {
        if ( _thumbs.count(fm) > 0)
            img = _thumbs.at(fm).clone();
        _rwlock.unlock();
    }   // end if
    return img;
}   // end thumbnail
*/


// static
QImage ActionUpdateThumbnail::generateImage( const FM *fm, const QSize &sz, float fov, float dscale)
{
    return generateImage( fm, fm->mesh(), sz, fov, dscale);
}   // end generateImage


// static
QImage ActionUpdateThumbnail::generateImage( const FM *fm, const r3d::Mesh &mesh,
                                                const QSize &sz, float fov, float dscale)
{
    // This function used to be an instance member function (now static). The note
    // below relates to when it was a member function:
    // "Don't make offscreen viewer a private member - VTK on Windows hangs in r3dvis::extractBGR
    // if exec in diff thread. Note that this issue doesn't arise if omv is a private member and
    // its snapshot function is called in the GUI thread."
    r3dvis::OffscreenMeshViewer omv( cv::Size(sz.width(), sz.height()));
    omv.setBackgroundColour( 1.0f, 1.0f, 1.0f);

    vtkActor *actor = omv.setModel( mesh);  // Create the actor in the viewer
    // Add normals for smooth lighting interpolation
    const auto rptr = FaceModelCurvatureStore::rvals( *fm);
    if ( rptr)
        r3dvis::getPolyData( actor)->GetPointData()->SetNormals( rptr->normals());

    vtkProperty *prop = actor->GetProperty();
    prop->SetInterpolationToPhong();
    if ( !mesh.hasMaterials())
    {
        static const QColor COL = Vis::FV::BASECOL;
        prop->SetColor( COL.redF(), COL.greenF(), COL.blueF());
    }   // end if

    omv.setCamera( ActionOrientCamera::makeFrontCamera( *fm, fov, dscale));
	const cv::Mat img = omv.snapshot(); // Breaks in separate thread if omv not constructed every time
	return QTools::copyOpenCV2QImage( img);
}   // end generateImage


void ActionUpdateThumbnail::setThumbnailSize( const QSize &sz) { _vsz = sz;}


bool ActionUpdateThumbnail::isAllowed( Event) { return MS::isViewSelected();}


bool ActionUpdateThumbnail::doBeforeAction( Event e) { return isAllowed(e);}


void ActionUpdateThumbnail::doAction( Event e)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    const QImage img = generateImage( fm.get(), _vsz, 30, 0.8f);
	fm->setThumbnail( QPixmap::fromImage(img));
	/*
    _rwlock.lockForWrite();
    _thumbs[fm.get()] = img;
    _rwlock.unlock();
	*/
}   // end doAction


Event ActionUpdateThumbnail::doAfterAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    emit updated( fm.get());
    return Event::NONE;
}   // end doAfterAction

/*
void ActionUpdateThumbnail::purge( const FM *fm)
{
    _rwlock.lockForWrite();
    _thumbs.erase(fm);
    _rwlock.unlock();
}   // end purge
*/
