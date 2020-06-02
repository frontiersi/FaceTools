/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

    bool attach( Vis::FV*);
    bool detach( Vis::FV*);

    const FVS& attached() const { return _attached;}
    bool isAttached( const FM* fm) const { return _models.count(fm) > 0;}
    Vis::FV* get( const FM* fm) const; // Pointer to view/control of given model or null if model not attached.

public slots:
    void saveScreenshot() const;

signals:
    void toggleZeroArea( bool); // When going from positve to zero viewing area (true) and back (false).
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
