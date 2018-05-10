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

#include <ActionGetFaceComponent.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionGetFaceComponent;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionGetFaceComponent::ActionGetFaceComponent( QString dn, QIcon icon)
    : FaceAction(true/*disable before other*/), _dname(dn), _icon(icon)
{
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
}   // end ctor


bool ActionGetFaceComponent::testReady( FaceControl* fc)
{
    return fc->data()->landmarks().has( FaceTools::Landmarks::NASAL_TIP);
}   // end testReady


bool ActionGetFaceComponent::doAction( FaceControlSet& rset)
{
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        // Get the single component attached to the face and set back in the model.
        int svidx = fm->kdtree()->find( fm->landmarks().pos( FaceTools::Landmarks::NASAL_TIP));
        RFeatures::ObjModel::Ptr face = FaceTools::getComponent( fm->model(), svidx);
        fm->setModel(face);
    }   // end for
    return true;
}   // end doAction
