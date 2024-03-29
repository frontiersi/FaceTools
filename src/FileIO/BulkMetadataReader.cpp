/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <FileIO/BulkMetadataReader.h>
#include <FileIO/FaceModelXMLFileHandler.h>
#include <QDir>
using FaceTools::FileIO::BulkMetadataReader;
using FaceTools::FM;

BulkMetadataReader::BulkMetadataReader( const QFileInfoList &files, bool withThumbs)
    : _files(files), _withThumbs(withThumbs), _docancel( false)
{
}   // end ctor


namespace {
QString readMetaData( const QString &fpath, FM &fm, bool withThumbs)
{
    PTree ptree;
    QPixmap *thumb = withThumbs ? new QPixmap : nullptr;
    QString err = FaceTools::FileIO::readMeta( fpath, ptree, thumb); // FaceModelXMLFileHandler.h
    static const QString ERRIN("[ERR] FaceTools::FileIO::BulkMetadataReader: %1");
    if ( !err.isEmpty())
        err = ERRIN.arg(err);
    double fversion = 0.0;
    if ( err.isEmpty() && !FaceTools::FileIO::importMetaData( fm, ptree, fversion))
        err = ERRIN.arg("Unable to FaceTools::FileIO::importMetaData from property tree!");
    if ( thumb && !thumb->isNull())
    {
        fm.setThumbnail( *thumb);
        delete thumb;
    }   // end if
    return err;
}   // end readMetaData
}   // end namespace


QList<FM*> BulkMetadataReader::_loadModels() const
{
    int i = 0;
    const int nfiles = _files.size();
    const float pfact = 100.0f / nfiles;
    QList<FM*> data;
    for ( const QFileInfo &pth : _files)
    {
        FM *fm = new FM;
        const QString err = readMetaData( pth.absoluteFilePath(), *fm, _withThumbs);
        if ( !err.isEmpty())
        {
            delete fm;
            fm = nullptr;
            std::cerr << err.toStdString() << std::endl;
        }   // end if
        data.append(fm);
        emit onPercentProgress( ++i * pfact);
        if ( _docancel)
            break;
    }   // end for
    return data;
}   // end _loadModels


void BulkMetadataReader::run()
{
    QList<FM*> data = _loadModels();
    if ( !_docancel)
        emit onLoadedModels( _files, data);
    else
    {
        emit onCancelled();
        _docancel = false;
    }   // end else
}   // end run


void BulkMetadataReader::cancel() { _docancel = true;}
