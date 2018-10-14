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

    bool applyToAllInViewer() const override { return true;}
    bool applyToSelectedModel() const override { return true;}

    bool belongs( const vtkProp*, const FV*) const override;
    bool isAvailable( const FM*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    void clear( FV*) override;

    // Show or hide the given landmark for the given FM.
    void setLandmarkVisible( const FM*, int, bool);

    // Set highlighted or not the given landmark for the given FM.
    void setLandmarkHighlighted( const FM*, int, FaceLateral, bool);

    // Set/update the view of the given landmark.
    void updateLandmark( const FM*, int);

    void refreshLandmarks( const FM*);

    // Return ID of landmark if given prop is for a landmark or -1 if not.
    // On return of >= 0, lat is set to the face lateral that the landmark appears.
    int landmarkId( const FV*, const vtkProp*, FaceLateral& lat) const;

    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;

    void purge( FV*) override;
    bool allowShowOnLoad( const FM* fm) const override;

private:
    std::unordered_map<const FV*, LandmarkSetView*> _views;
    bool hasView( const FV*) const;
};  // end class

}   // end namespace
}   // end namespace

#endif
