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

#include <LoadFaceModelsHelper.h>
#include <FaceModelManager.h>
#include <FaceModel.h>
#include <QMessageBox>
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceModel;


LoadFaceModelsHelper::LoadFaceModelsHelper( FaceModelManager* fmm, QWidget *parent)
    : _fmm(fmm), _parent(parent)
{
}   // end ctor


// public
size_t LoadFaceModelsHelper::setFilteredFilenames( const QStringList& fnames)
{
    // Get the supported filenames.
    QStringList notsupp;
    QStringList areopen;
    _filenames.clear();
    for ( const QString& qfname : fnames)
    {
        if ( !_fmm->canRead( qfname.toStdString()))
            notsupp << qfname;
        else if ( _fmm->isOpen( qfname.toStdString()))
            areopen << qfname;
        else
            _filenames << qfname;
    }   // end for

    // Check the load limit (assume that all selected can be loaded)
    if ( _filenames.size() + _fmm->numOpen() > _fmm->loadLimit())
    {
        const size_t nallowed = _fmm->loadLimit() - _fmm->numOpen();
        QString msg = QObject::tr("Too many files selected! Only ");
        msg.append( QString::number( _fmm->loadLimit())).append( QObject::tr(" models are allowed to be open at once."));
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


// public
size_t LoadFaceModelsHelper::loadModels()
{
    _failnames.clear();
    _loaded.clear();
    for ( const QString& fname : _filenames)
    {
        FaceModel* fm = _fmm->read( fname.toStdString());   // Blocks
        if ( fm)
            _loaded.push_back(fname.toStdString());
        else
            _failnames[_fmm->error().c_str()] << fname;
    }   // end for
    _filenames.clear();
    return _loaded.size();
}   // end loadModels


// public
bool LoadFaceModelsHelper::loadModel( const QString& filename)
{
    QStringList flist;
    flist << filename;
    if ( setFilteredFilenames( flist) < 1)
        return false;
    return loadModels() == 1;
}   // end loadModel


// public
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


// public
bool LoadFaceModelsHelper::reachedLoadLimit() const
{
    return _fmm->numOpen() >= _fmm->loadLimit();
}   // end reachedLoadLimit


QString LoadFaceModelsHelper::createImportFilters() const { return _fmm->fileFormats().createImportFilters();}
QStringList LoadFaceModelsHelper::createSimpleImportFilters() const { return _fmm->fileFormats().createSimpleImportFilters();}
