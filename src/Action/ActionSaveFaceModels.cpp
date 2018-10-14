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

#include <ActionSaveFaceModels.h>
#include <FaceModelManager.h>
#include <QMessageBox>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveFaceModels;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::FMM;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;


ActionSaveFaceModels::ActionSaveFaceModels( const QString& dn, const QIcon& ico, const QKeySequence& ks, QWidget *parent)
    : FaceAction( dn, ico, ks), _parent(parent)
{
    setAsync(true);
}   // end ctor


bool ActionSaveFaceModels::testReady( const FV* fv)
{
    FaceModel *fm = fv->data();
    return !fm->isSaved() && (FMM::hasPreferredFileFormat(fm) || !fm->hasMetaData());
}   // end testReady


bool ActionSaveFaceModels::doAction( FVS& fset, const QPoint&)
{
    const FaceModelSet& fms = fset.models();
    for ( FaceModel* fm : fms)
    {
        std::string filepath;   // Will be the last saved filepath
        if ( !FMM::write( fm, &filepath))  // Save using current filepath for the model
            _fails[FMM::error()] << filepath.c_str();
    }   // end for
    return true;
}   // end doAction


void ActionSaveFaceModels::doAfterAction( EventSet&, const FVS&, bool)
{
    for ( auto f : _fails)  // Display a critical error for each type of error message received
    {
        QString msg( (f.first + "\nUnable to save the following:\n").c_str());
        msg.append( f.second.join("\n"));
        QMessageBox::critical( _parent, tr("Unable to save file(s)!"), tr(msg.toStdString().c_str()));
    }   // end for
    _fails.clear(); // Ensure the fail set is cleared
}   // end doAfterAction
