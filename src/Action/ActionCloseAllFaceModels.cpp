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

#include <ActionCloseAllFaceModels.h>
#include <FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::FileIO::FMM;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( const QString& dname, QWidget* pw)
    : FaceAction( dname), _parent(pw)
{
}   // end ctor

bool ActionCloseAllFaceModels::testEnabled( const QPoint*) const { return FMM::numOpen() > 0;}

bool ActionCloseAllFaceModels::doBeforeAction( FVS& fvs, const QPoint&)
{
    bool doshowmsg = false;
    const FMS& models = FMM::opened();
    for ( FM* fm : models)
    {
        fm->lockForRead();
        if ( !fm->isSaved())
            doshowmsg = true;
        fm->unlock();
        if ( doshowmsg)
            break;
    }   // end for

    bool doclose = true;
    if ( doshowmsg)
    {
        static const QString msg = tr("Model(s) have unsaved changes! Close all anyway?");
        doclose = QMessageBox::Yes == QMessageBox::question( _parent, tr("Unsaved changes!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if

    if ( doclose)
    {
        for ( FM* fm : models)
            fvs.insert(fm);
    }   // end if

    return doclose;
}   // end doBeforeAction
