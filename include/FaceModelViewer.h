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

#ifndef FACE_TOOLS_FACE_MODEL_VIEWER_H
#define FACE_TOOLS_FACE_MODEL_VIEWER_H

/*
 * FaceModelViewer that allows for model(s) to be selected with duplicate
 * FaceViews of the same FaceModel disallowed.
 */

#include "ModelViewer.h"
#include "FaceViewSet.h"

namespace FaceTools {

using FVFlags = std::unordered_map<const Vis::FV*, bool>;

static const float DEFAULT_CAMERA_DISTANCE = 650.0f;

class FaceTools_EXPORT FaceModelViewer : public ModelViewer
{ Q_OBJECT
public:
    explicit FaceModelViewer( QWidget *parent=nullptr);

    bool attach( Vis::FV*);
    bool detach( Vis::FV*);

    const FVS& attached() const { return _attached;}
    bool isAttached( const Vis::FV* fv) const { return _attached.has(const_cast<Vis::FV*>(fv));}
    bool isAttached( const FM* fm) const { return _models.count(fm) > 0;}
    Vis::FV* get( const FM* fm) const; // Pointer to view/control of given model or null if model not attached.

    // Find out if any of the views attached to this viewer overlap with others; the given map
    // is set with true values for views that overlap. Returns the number of views that overlap
    // with others (i.e. the number of true settings in out parameter overlaps).
    // Out parameter is NOT cleared before setting.
    size_t findOverlaps( FVFlags& overlaps) const;

    // Given the overlaps set from a call to findOverlaps, refresh the opacity values of the views
    // attached to this viewer where a true entry in overlaps will result in the corresponding view's
    // opacity to be set to the minimum of its current value and maxOverlapOpacity.
    // The opacity values for non-overlapping views are set to 1.
    void refreshOverlapOpacity( const FVFlags& overlaps, double maxOverlapOpacity=0.5) const;

public slots:
    void resetCamera();
    void saveScreenshot() const;

signals:
    void toggleZeroArea( bool);             // When going from positve to zero viewing area (true) and back (false).
    void onAttached( Vis::FV*);
    void onDetached( Vis::FV*);

protected:
    void resizeEvent( QResizeEvent*) override;

private:
    FVS _attached; // All attached FaceControl instances
    std::unordered_map<const FM*, Vis::FV*> _models;
};  // end class

}   // end namespace

#endif
