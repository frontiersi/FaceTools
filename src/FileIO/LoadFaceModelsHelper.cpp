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

#include <LoadFaceModelsHelper.h>
#include <FaceModelManager.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <QMessageBox>
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FaceModel;

LoadFaceModelsHelper::LoadFaceModelsHelper( QWidget* parent) : _parent(parent) {}

int LoadFaceModelsHelper::setFilteredFilenames( const QStringList& fnames)
{
    // Get the supported filenames.
    QStringList notsupp;
    QStringList areopen;
    _filenames.clear();
    for ( const QString& qfname : fnames)
    {
        if ( !FMM::canRead( qfname.toStdString()))
            notsupp << qfname;
        else if ( FMM::isOpen( qfname.toStdString()))
            areopen << qfname;
        else
            _filenames << qfname;
    }   // end for

    // Check the load limit (assume that all selected can be loaded)
    if ( _filenames.size() + int(FMM::numOpen()) > int(FMM::loadLimit()))
    {
        const size_t nallowed = FMM::loadLimit() - FMM::numOpen();
        QString msg = QObject::tr("Too many files selected! Only ");
        msg.append( QString::number( FMM::loadLimit())).append( QObject::tr(" models are allowed to be open at once."));
        if ( nallowed == 1)
            msg.append( QObject::tr("Only one more model may be loaded."));
        else
            msg.append( QObject::tr("%1 more models may be loaded.").arg(nallowed));
        QMessageBox::warning( _parent, QObject::tr("Load limit reached!"), msg);
        _filenames.clear();
    }   // end if

    // Show warning for not supported files
    if ( notsupp.size() == 1)
        QMessageBox::warning( _parent, QObject::tr("Unsupported file type!"), notsupp.join(" ") + QObject::tr(" has an unsupported format!"));
    else if ( notsupp.size() > 1)
        QMessageBox::warning( _parent, QObject::tr("Unsupported file types!"), notsupp.join(", ") + QObject::tr(" are not supported file types!"));

    if ( areopen.size() == 1)
        QMessageBox::warning( _parent, QObject::tr("File already open!"), areopen.join(" ") + QObject::tr(" is already open!"));
    else if ( areopen.size() > 1)
        QMessageBox::warning( _parent, QObject::tr("Files already open!"), areopen.join(", ") + QObject::tr(" are already open!"));

    return _filenames.size();
}   // end setFilteredFilenames


size_t LoadFaceModelsHelper::loadModels()
{
    _loaded.clear();
    _failnames.clear();
    for ( const QString& fname : _filenames)
    {
        FaceModel* fm = FMM::read( fname.toStdString());   // Blocks
        if ( fm)
        {
            Vis::FV* fv = Action::ModelSelector::addFaceView(fm);
            Action::ModelSelector::setSelected(fv, true);
            _loaded.insert( fv);
        }   // end if
        else
            _failnames[FMM::error().c_str()] << fname;
    }   // end for
    _filenames.clear();
    return _loaded.size();
}   // end loadModels


bool LoadFaceModelsHelper::loadModel( const QString& filename)
{
    QStringList flist;
    flist << filename;
    if ( setFilteredFilenames( flist) < 1)
        return false;
    return loadModels() == 1;
}   // end loadModel


void LoadFaceModelsHelper::showLoadErrors()
{
    // For each error type, display a warning dialog
    for ( auto f : _failnames)
    {
        QString msg = f.first + QObject::tr("\nUnable to load the following:\n");
        msg.append( f.second.join("\n"));
        QMessageBox::warning( _parent, QObject::tr("Unable to load file(s)!"), msg);
    }   // end for
    _failnames.clear();
}   // end showLoadErrors


QString LoadFaceModelsHelper::createImportFilters() const { return FMM::fileFormats().createImportFilters();}
QStringList LoadFaceModelsHelper::createSimpleImportFilters() const { return FMM::fileFormats().createSimpleImportFilters();}
