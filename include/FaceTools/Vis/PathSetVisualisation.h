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

#ifndef FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H
#define FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H

#include "BaseVisualisation.h"
#include "PathSetView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathSetVisualisation : public BaseVisualisation
{
public:
    ~PathSetVisualisation() override;

    const char* name() const override { return "PathSetVisualisation";}

    bool applyToAllInViewer() const override { return false;}
    bool applyToAllViewers() const override { return false;}

    bool isAvailable( const FV*, const QPoint*) const override;

    float minAllowedOpacity() const override { return 0.1f;}
    float maxAllowedOpacity() const override { return 1.0f;}

    bool belongs( const vtkProp*, const FV*) const override;

    void apply( const FV*, const QPoint* mc=nullptr) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void addPath( const FM*, int pathId);
    void updatePath( const FM*, int pathId);
    void erasePath( const FM*, int pathId);
    void syncPaths( const FM*);

    // Returns the handle for a path by looking for it by prop.
    PathView::Handle* pathHandle( const FV*, const vtkProp*) const;
    // Returns handles 0 or 1 for the given path.
    PathView::Handle* pathHandle0( const FV*, int pathID) const;

    // Update text info from the specified path from the given model.
    // Text displayed at display coordinates x,y.
    void setText( const FM*, int pid, int x, int y);

    // Show the text caption just for the given view (turns off others).
    void showText( const Vis::FV* fv=nullptr);

    void refreshState( const FV*) override;
    void syncWithViewTransform( const FV*) override;

private:
    // The paths associated with a FV.
    std::unordered_map<const FV*, PathSetView*> _views;
    bool _hasView( const FV*) const;
};  // end class

}}   // end namespace

#endif
