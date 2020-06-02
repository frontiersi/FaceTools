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

#ifndef FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H
#define FACE_TOOLS_VIS_BOUNDING_VISUALISATION_H

#include "BaseVisualisation.h"
#include "BoundingView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BoundingVisualisation : public BaseVisualisation
{
public:
    ~BoundingVisualisation() override;

    const char* name() const override { return "BoundingVisualisation";}
    void apply( const FV*, const QPoint* mc=nullptr) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncWithViewTransform( const FV*) override;

private:
    std::unordered_map<const FV*, BoundingView*> _views;
    void _setAppearance( const FV*);
};  // end class

}}   // end namespace

#endif
