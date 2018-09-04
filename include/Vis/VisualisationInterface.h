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

#ifndef FACE_TOOLS_VIS_VISUALISATION_INTERFACE_H
#define FACE_TOOLS_VIS_VISUALISATION_INTERFACE_H

/**
 * The plugin interface for visualisation delegates passed in to ActionVisualise.
 * Declares a base set of interfacing functions that derived types must define.
 * Do not derive from this class directly - use BaseVisualisation since this
 * includes a bunch of necessary functions with default implementations.
 */

#include <FaceTypes.h>
#include <PluginInterface.h>    // QTools

namespace FaceTools {
namespace Vis {

// VisualisationInterface is pure virtual to allow it to be a plugin type.
class FaceTools_EXPORT VisualisationInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual bool isToggled() const = 0;                     // Toggled visualisations layer.
    virtual bool isAvailable( const FaceModel*) const = 0;  // Does the FaceModel allow this visualisation to be applied?
    virtual bool isAvailable( const FaceView*, const QPoint* mc=nullptr) const = 0; // Can visualise for view and mouse coords?

    virtual void copy( FaceView* dst, const FaceView* src) = 0;    // Copy visualisations directly across from src to dst.
    virtual void apply( FaceView*, const QPoint* mc=nullptr) = 0;  // Add the visualisation.
    virtual void remove( FaceView*) = 0;                           // Remove the visualisation.
};  // end class

}   // end namespace
}   // end namespace

#endif
