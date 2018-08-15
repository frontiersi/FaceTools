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

#ifndef FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H
#define FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H

/**
 * A visualisation bounding an ellipsoidal region across views of a model.
 */

#include "BaseVisualisation.h"
#include "LoopsView.h"
#include "SphereView.h"
#include <ObjModelRegionSelector.h> // RFeatures

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT RadialSelectVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    RadialSelectVisualisation( const QString& dname="Radial Select", const QIcon& icon=QIcon());
    ~RadialSelectVisualisation() override;

    bool isExclusive() const override { return false;}
    bool isAvailable( const FaceControl*, const QPoint* mp=nullptr) const override;

    bool belongs( const vtkProp*, const FaceControl*) const override;

    // Applies to all views of a single model.
    bool singleModel() const override { return true;}

    bool apply( const FaceControl*, const QPoint* mc=nullptr) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

    // Set pickability for the sphereView on the associated view (if it exists).
    void setPickable( const FaceControl*, bool);

    // Set whether the visualisation is highlighted or not. Returns true if highlighting changed.
    bool setHighlighted( const FaceControl*, bool);

    // Set/get new boundary radius/centre for the given FaceModel.
    void setCentre( const FaceModel*, const cv::Vec3f&);
    cv::Vec3f centre( const FaceModel*) const;

    // Set a new radius (cannot be set lower than MIN_RADIUS).
    void setRadius( const FaceModel*, double);
    double radius( const FaceModel*) const;

    static const double MIN_RADIUS;

protected:
    void pokeTransform( const FaceControl*, const vtkMatrix4x4*) override;
    void fixTransform( const FaceControl*) override;
    void purge( const FaceControl*) override;
    void purge( const FaceModel*) override;

private:
    void updateActors( const FaceModel*);
    void createActors( const FaceControl*);
    void resetRegionSelector( const FaceModel*, const cv::Vec3f&);
    std::unordered_map<const FaceControl*, LoopsView*>  _lviews;
    std::unordered_map<const FaceControl*, SphereView*> _sviews;
    std::unordered_map<const FaceModel*, RFeatures::ObjModelRegionSelector::Ptr> _rselectors;
    double _radius;
};  // end class

}   // end namespace
}   // end namespace

#endif
