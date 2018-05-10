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

#include <ActionFillHoles.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceModelViewer.h>
#include <ObjModelHoleFiller.h>
using FaceTools::Action::ActionFillHoles;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionFillHoles::ActionFillHoles( QProgressBar* pb)
    : FaceAction(true/*disable before other*/), _icon( ":/icons/FILL_HOLES")
{
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionFillHoles::testReady( FaceControl* fc) { return FaceTools::hasReqLandmarks(fc->data()->landmarks());}


bool ActionFillHoles::doAction( FaceControlSet& rset)
{
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();
    FaceModel* fm = fc->data();

    const int nfilled = RFeatures::ObjModelHoleFiller::fillHoles( fm->model()) - 1;
    if ( nfilled > 0)
        std::cerr << nfilled << " holes filled" << std::endl;
    else
        std::cerr << "No holes found!" << std::endl;
    fm->setModel(fm->model());
    return true;
}   // end doAction
