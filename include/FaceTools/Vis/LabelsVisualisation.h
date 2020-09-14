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

#ifndef FACE_TOOLS_VIS_LABELS_VISUALISATION_H
#define FACE_TOOLS_VIS_LABELS_VISUALISATION_H

#include "BaseVisualisation.h"
#include "LabelsView.h"
#include <FaceTools/FaceModelViewer.h>
#include <FaceTools/FaceModel.h>
#include <FaceTools.h>
#include <type_traits>

namespace FaceTools { namespace Vis {

template <class T>
class LabelsVisualisation : public BaseVisualisation
{
public:
    ~LabelsVisualisation() override;

    const char* name() const override { return "LabelsVisualisation";}
    bool isAvailable( const FV*, const QPoint*) const override;

    void apply( const FV*, const QPoint* mc=nullptr) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncWithViewTransform( const FV*) override;

    void refresh( const FV*) override;

private:
    std::unordered_map<const FV*, T> _views;

    static_assert( std::is_base_of<LabelsView, T>::value, "Value type must derive from LabelsView!");
};  // end class

#include "LabelsVisualisation.cpp"

}}   // end namespace

#endif
