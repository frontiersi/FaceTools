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

#ifndef FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H
#define FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H

#include "BaseVisualisation.h"
#include "BoundingView.h"

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT BoundingVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    explicit BoundingVisualisation( const QString& dname="BoundingVisualisation");
    ~BoundingVisualisation() override;

    bool isUIVisible() const override { return false;}

    // Used to bound just the selected views.
    bool applyToAllInViewer() const override { return false;}

    void apply( FV*, const QPoint* mc=nullptr) override;
    void remove( FV*) override;

    // Set the given component to be highlighted or not.
    // If c == -1, then all components are referenced.
    void setHighlighted( const FV*, int c=-1, bool v=false);

    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;

protected:
    void purge( FV*) override;
    bool applyOnReady() const override { return true;}

private:
    std::unordered_map<const FV*, BoundingView*> _views;
};  // end class

}   // end namespace
}   // end namespace

#endif
