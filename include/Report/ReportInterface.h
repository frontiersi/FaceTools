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

#ifndef FACE_TOOLS_REPORT_INTERFACE_H
#define FACE_TOOLS_REPORT_INTERFACE_H

#include <FaceTypes.h>
#include <PluginInterface.h>    // QTools

namespace FaceTools {
namespace Report {

// ReportInterface is pure virtual to allow it to be a plugin type.
class FaceTools_EXPORT ReportInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual bool isAvailable( const FaceModel*) const = 0;  // If the data for this report are available.
};  // end class

}   // end namespace
}   // end namespace

#endif
