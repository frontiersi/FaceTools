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

#ifndef FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H
#define FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H

#include "BaseVisualisation.h"
#include "RadialSelectView.h"

namespace FaceTools {

namespace Interactor {
class RadialSelectHandler;
}   // end namespace

namespace Vis {

class FaceTools_EXPORT RadialSelectVisualisation : public BaseVisualisation
{
public:
    RadialSelectVisualisation();
    ~RadialSelectVisualisation() override;

    void setHandler( const Interactor::RadialSelectHandler *h) { _handler = h;}

    const char* name() const override { return "RadialSelectVisualisation";}

    bool applyToAllInViewer() const override { return false;}
    bool applyToAllViewers() const override { return true;}
    bool isAvailable( const FV*, const QPoint*) const override;
    bool belongs( const vtkProp*, const FV*) const override;

    void apply( const FV*, const QPoint* mc=nullptr) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void refreshState( const FV*) override;
    void syncWithViewTransform( const FV*) override;

    // Set whether the visualisation is highlighted or not.
    void setHighlighted( const FV*, bool);

private:
    const Interactor::RadialSelectHandler *_handler;
    std::unordered_map<const FV*, RadialSelectView> _views;
};  // end class

}}   // end namespaces

#endif
