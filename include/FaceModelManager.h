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

#ifndef FACE_TOOLS_FACE_MODEL_MANAGER_H
#define FACE_TOOLS_FACE_MODEL_MANAGER_H

#include <QObject>
#include <QStringList>
#include <PluginsLoader.h>   // QTools
#include <PluginInterface.h> // QTools
#include "FileIOInterface.h"
#include "FaceModel.h"

namespace FaceTools
{

class FaceTools_EXPORT FaceModelManager : public QObject
{ Q_OBJECT
public:
    typedef boost::shared_ptr<FaceModelManager> Ptr;
    static FaceModelManager::Ptr get(); // Singleton

    // Loads a FaceModel depending on file extension asynchronously and fires finishedIO when done.
    // Returns a non empty string containing error info if filename invalid (doesn't exist or not a valid file type).
    std::string load( const std::string& fname);

    // Saves model asynchronously and fires finishedIO when done.
    // Returns non empty string containing error info if fname invalid.
    std::string save( FaceModel*, const std::string& fname);


    bool close( const FaceModel*);  // Close, returning true
    void closeAll(); // Closes all currently opened models

    bool isValidImportFilename( const std::string& fname);
    bool isValidExportFilename( const std::string& fname);
    void printIOFormats( std::ostream&);

    // Use return value with QFileDialog.setNameFilters
    QStringList getImportFilters() { return _importFilters;}
    QStringList getExportFilters() { return _exportFilters;}

    QString getFilter( const std::string& ext);  // Gets file dialog filter for extension (empty if invalid ext)
    const std::string & getPrimaryExt() const { return _primaryExt;}

    void setPluginsLoader( QTools::PluginsLoader*);

signals:
    void finishedIO( FaceModel*, const std::string&);

private slots:
    void doOnLoadedPlugin( QTools::PluginInterface*);
    void postProcessIO( FaceModel*, const std::string&);

private:
    boost::unordered_map<std::string, FileIOInterface*> _fileInterfaces;
    boost::unordered_map<std::string, std::string> _importExtDescMap; // Maps extensions to descriptions
    boost::unordered_map<std::string, std::string> _exportExtDescMap; // Maps extensions to descriptions
    QStringList _importFilters, _exportFilters;
    void populatePlugins( FileIOInterface*);

    std::string _primaryExt;
    boost::unordered_set<const FaceModel*> _fmodels;   // Open models

    static FaceModelManager::Ptr s_fmm;

    FaceModelManager();
    ~FaceModelManager();
    FaceModelManager( const FaceModelManager&);
    FaceModelManager& operator=( const FaceModelManager&);
    class Deleter;
};  // end class

}   // end namespace FaceApp

#endif

