/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_METRIC_REPORT_MANAGER_H
#define FACE_TOOLS_METRIC_REPORT_MANAGER_H

#include "Report.h"

namespace FaceTools { namespace Report {

class FaceTools_EXPORT ReportManager
{
public:
    // Initialise PDF export with the locations of pdflatex and IDTFConverter.
    static bool init( const QString& pdflatex, const QString& idtfConverter);

    // Returns whether reporting is available.
    static bool isAvailable();

    // Load all report Lua scripts from the given directory.
    static int load( const QString&);

    // Add a single report from the given filepath returning its identifying
    // name or an empty string if the report couldn't be added.
    static QString add( const QString&);

    // Return the number of reports.
    static size_t count() { return _reports.size();}

    // Returns alphanumerically sorted list of report names.
    static const QStringList& names() { return _names;}

    // Return reference to the report with given name or null if doesn't exist.
    static Report::Ptr report( const QString& nm) { return _reports.count(nm) > 0 ? _reports.at(nm) : nullptr;}

private:
    static QStringList _names;                                  // Report names
    static std::unordered_map<QString, Report::Ptr> _reports;   // Reports keyed by their names
};  // end class

}}  // end namespaces

#endif
