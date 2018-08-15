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

#ifndef FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H
#define FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H

#include "BaseVisualisation.h"
#include "LandmarkSetView.h"

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LandmarksVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    LandmarksVisualisation( const QString &dname, const QIcon &icon);
    ~LandmarksVisualisation() override;

    bool isExclusive() const override { return false;}
    bool singleModel() const override { return true;}    // Apply to all FaceControls of a FaceModel
    bool belongs( const vtkProp*, const FaceControl*) const override;

    bool apply( const FaceControl*, const QPoint* mc=nullptr) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

    // Show or hide the given landmark for the given FaceControl.
    void setLandmarkVisible( const FaceModel*, int, bool);

    // Set highlighted or not the given landmark for the given FaceControl.
    void setLandmarkHighlighted( const FaceModel*, int, bool);

    // Refresh information about the given landmark from its set for the given FaceControl.
    void updateLandmark( const FaceModel*, int);

    // Return ID of landmark if given prop is for a landmark or -1 if not.
    int landmarkProp( const FaceControl*, const vtkProp*) const;

protected:
    void pokeTransform( const FaceControl*, const vtkMatrix4x4*) override;
    void fixTransform( const FaceControl*) override;
    void purge( const FaceControl*) override;

private:
    std::unordered_map<const FaceControl*, LandmarkSetView*> _views;
};  // end class

}   // end namespace
}   // end namespace

#endif
