/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

class FaceTools_EXPORT FaceModelViewer : public ModelViewer
{ Q_OBJECT
public:
    explicit FaceModelViewer( QWidget *parent=nullptr);

    bool attach( Vis::FV*); // Also makes the given view the selected one
    bool detach( Vis::FV*);

    bool isZeroArea() const;

    inline bool empty() const { return _attached.empty();}
    inline const FVS& attached() const { return _attached;}
    inline bool has( const Vis::FV *fv) const { return _attached.has(fv);}
    inline bool isAttached( const FM* fm) const { return _models.count(fm) > 0;}
    Vis::FV* get( const FM* fm) const; // Pointer to view of given model or null if not attached.

    // Return the other view that *isn't* the given view or null
    // if there are not exactly two FaceViews in this viewer.
    Vis::FV* other( const Vis::FV*) const;

    inline void setSelected( Vis::FV *fv) { _lastfv = fv;}
    inline Vis::FV *selected() const { return _lastfv;}

signals:
    void toggleZeroArea( bool); // When going from positve to zero viewing area (true) and back (false).
    void onAttached( Vis::FV*);
    void onDetached( Vis::FV*);

protected:
    void resizeEvent( QResizeEvent*) override;

private:
    FVS _attached;
    std::unordered_map<const FM*, Vis::FV*> _models;
    Vis::FV *_lastfv;
};  // end class

}   // end namespace

#endif
