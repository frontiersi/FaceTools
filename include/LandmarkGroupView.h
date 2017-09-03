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

#ifndef FACE_TOOLS_LANDMARK_GROUP_VIEW_H
#define FACE_TOOLS_LANDMARK_GROUP_VIEW_H

#include "ObjMetaData.h"
#include "LandmarkView.h"


namespace FaceTools
{

class FaceTools_EXPORT LandmarkGroupView : public QObject
{ Q_OBJECT
public:
    LandmarkGroupView( ModelViewer*, const ObjMetaData::Ptr);
    virtual ~LandmarkGroupView();

    void showAll( bool enable);                     // Show/hide all landmarks
    bool isShown() const;                           // Returns true iff ALL landmarks shown
    void showLandmark( const std::string&, bool);   // Show/hide individual landmark
    bool isShown( const std::string&) const;        // Returns true iff a particular landmark is shown

    void highlightLandmark( const std::string&, bool);
    void setVisualisationOptions( const VisualisationOptions::Landmarks&);

    void erase(); // Remove existing landmarks
    void reset(); // Reset all landmarks from the ObjMetaData

    // Returns the name of the landmark under the given coordinates
    // or an empty string if no landmarks lie under the coordinates.
    std::string pointedAt( const QPoint&) const;

public slots:
    void updateLandmark( const std::string&, const cv::Vec3f*);
    void selectLandmark( const std::string&, bool enable);

private:
    ModelViewer* _viewer;
    const ObjMetaData::Ptr _objmeta;
    boost::unordered_map<std::string, LandmarkView*> _lviews;
    VisualisationOptions::Landmarks _visopts;
};  // end class

}   // end namespace

#endif
