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

#include <ActionLoadDirFaceModels.h>
#include <FaceModelManager.h>
#include <QFileDialog>
using FaceTools::Action::ActionLoadDirFaceModels;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FVS;


ActionLoadDirFaceModels::ActionLoadDirFaceModels( const QString& dn, const QIcon& ico, QWidget *p)
    : FaceAction( dn, ico), _loadHelper(p)
{
    setAsync(true);
}   // end ctor


bool ActionLoadDirFaceModels::testEnabled( const QPoint*) const
{
    using FaceTools::FileIO::FMM;
    return FMM::numOpen() < FMM::loadLimit();
}   // end testEnabled


bool ActionLoadDirFaceModels::doBeforeAction( FVS&, const QPoint&)
{
    QString dname = QFileDialog::getExistingDirectory( _loadHelper.parent(),
                                                       tr("Select directory containing models"), "",
                                                       QFileDialog::ShowDirsOnly);
    QDir qdir(dname); // Get list of filenames from directory
    QStringList fnames = qdir.entryList( _loadHelper.createSimpleImportFilters());
    std::for_each( std::begin(fnames), std::end(fnames), [&](QString& fn){ fn = QDir::cleanPath( qdir.filePath(fn));});
    return _loadHelper.setFilteredFilenames( fnames) > 0;
}   // end doBeforeAction


bool ActionLoadDirFaceModels::doAction( FVS& fvs, const QPoint&)
{
    fvs.clear();
    if ( _loadHelper.loadModels() > 0)  // blocks
        fvs.insert(_loadHelper.lastLoaded());
    return !fvs.empty();
}   // end doAction


void ActionLoadDirFaceModels::doAfterAction( EventSet& cs, const FVS&, bool loaded)
{
    _loadHelper.showLoadErrors();
    if ( loaded)
        cs.insert(LOADED_MODEL);
}   // end doAfterAction
