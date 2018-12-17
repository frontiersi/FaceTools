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

#ifndef FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H
#define FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H

#include "BaseVisualisation.h"
#include "PathSetView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathSetVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    PathSetVisualisation( const QString &dname, const QIcon &icon);
    ~PathSetVisualisation() override;

    bool applyToAllInViewer() const override { return true;}
    bool applyToSelectedModel() const override { return true;}

    bool belongs( const vtkProp*, const FV*) const override;
    bool isAvailable( const FM*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    void clear( FV*) override;

    // Creates and shows the given path across all FaceViews for the FaceModel.
    void addPath( const FM*, int pathId);

    // Update the position of the handles for the given path.
    void updatePath( const FM*, int pathId);

    // Refresh visualisation of paths for the given FaceModel.
    void refresh( const FM*);

    // Returns the handle for a path by looking for it by prop.
    PathView::Handle* pathHandle( const FV*, const vtkProp*) const;
    // Returns handles 0 or 1 for the given path.
    PathView::Handle* pathHandle0( const FV*, int pathID) const;
    PathView::Handle* pathHandle1( const FV*, int pathID) const;

    // Update text info from the specified path from the given model.
    // Text displayed at display coordinates x,y.
    void setText( const FM*, int pid, int x, int y);

    // Show the text caption for all views of the associated model - turning off all others.
    void showText( const FM* fm=nullptr);

    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;

    void purge( FV*) override;

private:
    // The paths associated with a FV.
    std::unordered_map<const FV*, PathSetView*> _views;
    bool hasView( const FV*) const;
};  // end class

}}   // end namespace

#endif
