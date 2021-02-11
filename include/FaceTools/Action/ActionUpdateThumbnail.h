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

#ifndef FACE_TOOLS_ACTION_ACTION_UPDATE_THUMBNAIL_H
#define FACE_TOOLS_ACTION_ACTION_UPDATE_THUMBNAIL_H

#include "FaceAction.h"
#include <QReadWriteLock>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionUpdateThumbnail : public FaceAction
{ Q_OBJECT
public:
    ActionUpdateThumbnail( int width=256, int height=256);

    void setThumbnailSize( const QSize&);

    // Returns thumbnail for the given model or the currently selected one if null.
    static cv::Mat thumbnail( const FM *fm=nullptr);

    // Generate an image of the given model having given size. Params fov and
    // dscale give the camera field of view and distance (scale) from focus
    // (passed through to ActionOrientCamera::makeFrontCamera).
    static cv::Mat generateImage( const FM*, const QSize&, float fov=30, float dscale=1.0f);

    // As above but pass a different mesh (which might have a different texture).
    // The mesh must have the same geometry (or at least the same face IDs) for assigning normals.
    static cv::Mat generateImage( const FM*, const r3d::Mesh&, const QSize&, float fov=30, float dscale=1.0f);

signals:
    // Emitted whenever a new thumbnail generated for the currently selected model.
    void updated();

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;
    void purge( const FM*) override;

private:
    QSize _vsz;
    static std::unordered_map<const FM*, cv::Mat_<cv::Vec3b> > _thumbs;
    static QReadWriteLock _rwlock;
};  // end class

}}   // end namespaces

#endif
