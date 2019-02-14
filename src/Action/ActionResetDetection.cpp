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

#include <ActionResetDetection.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionResetDetection;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionResetDetection::ActionResetDetection( const QString& dn, const QIcon& icon, QWidget *parent)
    : FaceAction(dn, icon), _parent(parent)
{
}   // end ctor


bool ActionResetDetection::testReady( const FV* fv) { return !fv->data()->landmarks()->empty();}

bool ActionResetDetection::testEnabled( const QPoint*) const { return ready1();}


bool ActionResetDetection::doBeforeAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    static const QString msg = tr("Really reset existing landmark and metric detections?");
    bool go = QMessageBox::Yes == QMessageBox::question( _parent, tr("Reset face detection?"), msg,
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return go;
}   // end doBeforeAction


bool ActionResetDetection::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FM* fm = fvs.first()->data();
    fm->lockForWrite();
    fm->clearMeta();
    fm->landmarks()->clear();
    // Transform back to original position
    const RFeatures::Orientation& on = fm->orientation();
    const cv::Vec3f& c = fm->centre();
    cv::Matx44d m = RFeatures::toStandardPosition( on.nvec(), on.uvec(), c);
    fm->transform(m);
    fm->unlock();
    fvs.insert(fm);
    return true;
}   // end doAction


void ActionResetDetection::doAfterAction( EventSet& cset, const FVS&, bool)
{
    cset.insert(ORIENTATION_CHANGE);
    cset.insert(LANDMARKS_DELETE);
    cset.insert(AFFINE_CHANGE);
}   // end doAfterAction
