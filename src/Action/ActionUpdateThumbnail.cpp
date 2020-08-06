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

#include <Action/ActionUpdateThumbnail.h>
#include <Vis/FaceView.h>
#include <FaceModelCurvature.h>
#include <FaceModel.h>
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionUpdateThumbnail::ActionUpdateThumbnail( int w, int h)
    : FaceAction("Thumbnail Updater"), _omv( cv::Size(w,h))
{
    addTriggerEvent( Event::ASSESSMENT_CHANGE);
    addTriggerEvent( Event::MESH_CHANGE);
    addTriggerEvent( Event::MODEL_SELECT);
}   // end ctor


ActionUpdateThumbnail::~ActionUpdateThumbnail()
{
    while ( !_thumbs.empty())
        _thumbs.erase( _thumbs.begin()->first);
}   // end dtor


bool ActionUpdateThumbnail::checkState( Event)
{
    QColor bgcol = MS::defaultViewer()->backgroundColour();
    _omv.setBackgroundColour( bgcol.redF(), bgcol.greenF(), bgcol.blueF());
    return true;
}   // end checkState


const cv::Mat ActionUpdateThumbnail::thumbnail( const FM* fm)
{
    if ( _thumbs.count(fm) > 0)
        return _thumbs.at(fm);

    fm->lockForRead();
    vtkActor *actor = _omv.setModel( fm->mesh());
    if ( !fm->mesh().hasMaterials())
    {
        _omv.setModelColour( Vis::FV::BASECOL.redF(), Vis::FV::BASECOL.greenF(), Vis::FV::BASECOL.blueF());
        setNormals( actor, fm);
    }   // end if
    const Mat4f& T = fm->transformMatrix();
    fm->unlock();

    const Vec3f uvec = T.block<3,1>(0,1);
    const Vec3f nvec = T.block<3,1>(0,2);
    const Vec3f cent = T.block<3,1>(0,3);
    float dist = 400.0f;
    if ( fm->hasLandmarks())
        dist = sqrtf(fm->currentLandmarks().sqRadius()) * 3.5;
    const Vec3f cpos = dist * nvec + cent;

    const r3d::CameraParams cam( cpos, cent, uvec, 30);
    _omv.setCamera( cam);
    return _thumbs[fm] = _omv.snapshot();
}   // end thumbnail


void ActionUpdateThumbnail::doAction( Event)
{
    const FM* fm = MS::selectedModel();
    _thumbs.erase(fm);
    const cv::Mat img = thumbnail(fm);
    emit updated( fm, img);
}   // end doAction


bool ActionUpdateThumbnail::isAllowed( Event) { return MS::isViewSelected();}


void ActionUpdateThumbnail::purge( const FM* fm) { _thumbs.erase(fm);}
