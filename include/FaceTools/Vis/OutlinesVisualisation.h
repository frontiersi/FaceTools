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

#ifndef FACE_TOOLS_VIS_OUTLINES_VISUALISATION_H
#define FACE_TOOLS_VIS_OUTLINES_VISUALISATION_H

#include "BaseVisualisation.h"
#include "LoopView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT OutlinesVisualisation : public BaseVisualisation
{
public:
    OutlinesVisualisation();

    bool applyToAllInViewer() const override { return false;}
    bool applyToAllViewers() const override { return false;}

    void refresh( FV*) override;
    void purge( const FV*) override;
    void purgeAll();

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncTransform( const FV*) override;

    void setManifoldIndex( int);

private:
    std::unordered_map<const FV*, std::vector<LoopView> > _views;
    int _midx;
};  // end class

}}   // end namespace

#endif
