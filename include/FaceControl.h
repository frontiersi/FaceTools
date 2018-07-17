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

#ifndef FACE_TOOLS_FACE_CONTROL_H
#define FACE_TOOLS_FACE_CONTROL_H

/**
 * Container that associates a model to a FaceView (and its visualisations).
 * There can be many FaceControl instances associated with a single FaceModel.
 * Provides convenience functions for coordinating transformations between
 * the view and data models.
 */

#include "FaceTools_Export.h"
#include <opencv2/opencv.hpp>
#include <QMetaType>

namespace FaceTools {
class FaceModelViewer;
class FaceModel;

namespace Vis { class FaceView;}

class FaceTools_EXPORT FaceControl
{
public:
    FaceControl() : _fdata(nullptr), _fview(nullptr) {} // For QMetaType
    FaceControl( FaceModel*, FaceModelViewer*);
    virtual ~FaceControl();

    // Convenience function to get/set viewer this FaceControl uses.
    FaceModelViewer* viewer() const;
    void setViewer( FaceModelViewer*);

    inline FaceModel* data() const { return _fdata;}
    inline Vis::FaceView* view() const { return _fview;}

private:
    FaceModel *_fdata;
    Vis::FaceView *_fview;
};  // end class

}   // end namespace

Q_DECLARE_METATYPE( FaceTools::FaceControl)
    
#endif
