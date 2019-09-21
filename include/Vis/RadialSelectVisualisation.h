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

#ifndef FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H
#define FACE_TOOLS_VIS_RADIAL_SELECT_VISUALISATION_H

#include "BaseVisualisation.h"
#include "SphereView.h"
#include "PointsView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT RadialSelectVisualisation : public BaseVisualisation
{
public:
    explicit RadialSelectVisualisation( double sphereRad=4.0);
    ~RadialSelectVisualisation() override;

    bool applyToAllInViewer() const override { return false;}
    bool applyToSelectedModel() const override { return true;}

    bool belongs( const vtkProp*, const FV*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    bool purge( FV*, Action::Event) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncToViewTransform( const FV* fv, const vtkMatrix4x4*) override;

    // Set new reticule position and loop vertex ids for the given model.
    void set( const FM*, const cv::Vec3f&, const IntSet*);

    // Set whether the visualisation is highlighted or not.
    void setHighlighted( const FM*, bool);

private:
    class View : public ViewInterface
    {
    public:
        View();
        virtual ~View();
        void reset();
        void set( const FV*, const cv::Vec3f&, const IntSet*, double);

        void setColour( double r, double g, double b, double a) override;
        void setVisible( bool, ModelViewer*) override;
        bool visible() const override;
        void pokeTransform( const vtkMatrix4x4*) override;

        void setPickable( bool);
        void setHighlighted( bool);

        SphereView *centre;
        PointsView *points;
        const IntSet* vidxs;
    };  // end class

    const double _srad;
    std::unordered_map<const FV*, View*> _views;
};  // end class

}}   // end namespaces

#endif
