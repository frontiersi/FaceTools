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

#ifndef FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H
#define FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H

#include "BaseVisualisation.h"
#include "BoundingView.h"
#include <unordered_map>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT BoundingVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    explicit BoundingVisualisation( const QString& dname="BoundingVisualisation");
    ~BoundingVisualisation() override;

    bool isExclusive() const override { return false;}

    bool isVisible() const override { return false;}

    // Used to bound the selected view - so should only be applied to a single FaceView.
    bool singleView() const override { return true;}

    void apply( const FaceControl*) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

protected:
    void pokeTransform( const FaceControl*, const vtkMatrix4x4*) override;
    void fixTransform( const FaceControl*) override;
    void purge( const FaceControl*) override;
    bool applyOnReady() const override { return true;}

private:
    std::unordered_map<const FaceControl*, BoundingView*> _views;
};  // end class

}   // end namespace
}   // end namespace

#endif
