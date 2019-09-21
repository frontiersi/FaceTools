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

#include <Action/ActionSlice.h>
#include <FaceModel.h>
#include <ObjModelTools.h>
using FaceTools::Action::ActionSlice;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

using RFeatures::ObjModel;
using RFeatures::ObjModelSlicer;
using RFeatures::Transformer;


ActionSlice::ActionSlice( const QString &dn, const QIcon& ico, const cv::Vec3f& p, const cv::Vec3f& n)
    : FaceAction( dn, ico), _p(p), _n(n)
{
    setAsync(true);
}   // end ctor


bool ActionSlice::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


void ActionSlice::doAction( Event)
{
    storeUndo( this, {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});
    FM* fm = MS::selectedModel();

    fm->lockForRead();
    ObjModel::Ptr nmod = ObjModelSlicer( fm->model())( _p, _n);    // One half
    fm->unlock();

    fm->lockForWrite();
    fm->update(nmod);
    fm->moveLandmarksToSurface();
    fm->unlock();
}   // end doAction


void ActionSlice::doAfterAction( Event)
{
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});
}   // end doAfterAction

