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

#ifndef FACE_TOOLS_VISUALISATIONS_MANAGER_H
#define FACE_TOOLS_VISUALISATIONS_MANAGER_H

/**
 * Used internally by FaceActionManager.
 */

#include "FaceAction.h"
#include <BaseVisualisation.h>
#include <QActionGroup>

namespace FaceTools {
namespace Action {
class FaceActionManager;

class FaceTools_EXPORT VisualisationsManager : public QObject
{ Q_OBJECT
public:
    VisualisationsManager();

    // Return a concatenation of the exclusive and non-exclusive actions with a dividing separator.
    // If there are no non-exclusive actions, no separator is added and only a list of the
    // exclusive actions is returned (which may be empty).
    QList<QAction*> actions() const;

private:
    // Constructs Texture, Surface, Wireframe, and Points visualisations by default.
    void makeDefault( FaceActionManager*);

    void add( FaceAction*); // Does nothing if parameter cannot be cast to ActionVisualise.

    // Try to apply the texture visualisation first, and surface visualisation second.
    // Returns true iff either one was set okay.
    bool setDefaultVisualisation( Vis::FaceView*);

    // If any of the FaceControls associated with the given models set don't have an
    // exclusive visualisation set, then set the default. In addition, if any FaceControls
    // in the same viewer have an inconsistent set of visualisations, set the default for all.
    void enforceVisualisationConformance( const FaceControlSet*);

    friend class FaceActionManager;

    QActionGroup _evis;     // Exclusive visualisation actions
    QActionGroup _nvis;     // Non-exclusive visualisation actions
    std::unordered_set<FaceAction*> _actions;

    Vis::TextureVisualisation _tvis;
    Vis::SurfaceVisualisation _svis;
    Vis::WireframeVisualisation _wvis;
    Vis::PointsVisualisation _pvis;

    VisualisationsManager( const VisualisationsManager&) = delete;
    void operator=( const VisualisationsManager&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
