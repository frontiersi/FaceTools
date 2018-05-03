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

#ifndef FACE_TOOLS_VIS_VISUALISATION_INTERFACE_H
#define FACE_TOOLS_VIS_VISUALISATION_INTERFACE_H

/**
 * The plugin interface for visualisation delegates passed in to ActionVisualise.
 * Declares a base set of interfacing functions that derived types must define.
 * Do not derive from this class directly - use BaseVisualisation since this
 * includes a bunch of necessary functions with default implementations.
 */

#include <FaceTools_Export.h>
#include <PluginInterface.h>    // QTools

namespace FaceTools {
class FaceModel;
class FaceControl;
class ModelViewer;

namespace Vis {

// VisualisationInterface is pure virtual to allow it to be a plugin type.
class FaceTools_EXPORT VisualisationInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual bool isExclusive() const = 0;                   // Exclusive visualisations cannot be layered.
    virtual bool isAvailable( const FaceModel*) const = 0;  // If the data for this visualisation are available.

    virtual void apply( const FaceControl*) = 0;            // Apply visualisation post-processing.
    virtual void addActors( const FaceControl*) = 0;        // Add the visualisation specific actors.
    virtual void removeActors( const FaceControl*) = 0;     // Remove the view actors from viewer.
};  // end class

}   // end namespace
}   // end namespace

#define FaceToolsPluginVisVisualisationInterface_iid "com.github.richeytastic.FaceTools.v030.Vis.VisualisationInterface"
Q_DECLARE_INTERFACE( FaceTools::Vis::VisualisationInterface, FaceToolsPluginVisVisualisationInterface_iid)

#endif
