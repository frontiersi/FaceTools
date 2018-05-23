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
#include "SphereView.h"

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT BoundingVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    BoundingVisualisation( const QString& dname="Bounding",
                           const QIcon& icon=QIcon(":/icons/BOUNDARY"));
    ~BoundingVisualisation() override;

    bool isExclusive() const override { return false;}
    bool isAvailable( const FaceModel*) const override;

    void apply( const FaceControl*) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

    // Set/get a new radius of the boundary for the given FaceControl.
    void setCentre( const FaceControl*, const cv::Vec3f&);
    cv::Vec3f centre( const FaceControl*) const;

    // Set/get a new boundary centre for the given FaceControl.
    void setRadius( const FaceControl*, double);
    double radius( const FaceControl*) const;

protected:
    void transform( const FaceControl*, const vtkMatrix4x4*) override;
    void purge( const FaceControl*) override;

private:
    std::unordered_map<const FaceControl*, SphereView*> _views;
};  // end class

}   // end namespace
}   // end namespace

#endif
