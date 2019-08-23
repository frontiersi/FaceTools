/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <OffscreenModelViewer.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionUpdateThumbnail : public FaceAction
{ Q_OBJECT
public:
    ActionUpdateThumbnail( int width=256, int height=256);
    ~ActionUpdateThumbnail() override;

    void setThumbnailSize( int w, int h) { _omv.setSize(cv::Size(w,h));}

    // Returns thumbnail for the given model - generates if not already available.
    const cv::Mat thumbnail( const FM*);

signals:
    void updated( const FM*, const cv::Mat&);   // Emitted whenever a new thumbnail generated for the model.

protected:
    void purge( const FM*, Event) override;
    bool checkState( Event) override;
    void doAction( Event) override;

private:
    RVTK::OffscreenModelViewer _omv;
    std::unordered_map<const FM*, cv::Mat_<cv::Vec3b> > _thumbs;
};  // end class

}}   // end namespaces

#endif
