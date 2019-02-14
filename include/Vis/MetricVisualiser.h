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

#ifndef FACE_TOOLS_VIS_METRIC_VISUALISER_H
#define FACE_TOOLS_VIS_METRIC_VISUALISER_H

#include <BaseVisualisation.h>
#include <vtkTextActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT MetricVisualiser : public BaseVisualisation
{ Q_OBJECT
public:
    explicit MetricVisualiser( int);    // Supply the associated metric ID
    ~MetricVisualiser() override;

    int id() const { return _id;}

    /*
    bool belongs( const vtkProp*, const FV*) const;
    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;
    bool isAvailable( const FM*) const override;
    */

    void apply( FV*, const QPoint*) override;
    void clear( FV*) override;
    void purge( FV*) override;

    // Remeasure and update text content across associated views.
    void updateText( const FM*);

    // Show metric text for all views of the given model, or hide all if null.
    void showText( const FM* fm=nullptr);

    virtual void setHighlighted( const FM* fm=nullptr) = 0;

private:
    int _id;
    std::unordered_map<const FV*, vtkTextActor*> _texts;
};  // end class

}}   // end namespaces

#endif
