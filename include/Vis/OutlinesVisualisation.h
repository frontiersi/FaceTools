/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <unordered_map>

namespace FaceTools {
namespace Vis {
class OutlinesView;

class FaceTools_EXPORT OutlinesVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    OutlinesVisualisation( const QString &dname, const QIcon &icon, const QKeySequence &keys);
    ~OutlinesVisualisation() override;

    bool isExclusive() const override { return false;}

    void apply( const FaceControl*) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

protected:
    bool respondData() const override { return true;}
    void respondTo( const Action::FaceAction*, const FaceControl*) override;
    void transform( const FaceControl*, const vtkMatrix4x4*) override;
    void purge( const FaceControl*) override;

private:
    std::unordered_map<const FaceControl*, OutlinesView*> _views;
};  // end class

}   // end namespace
}   // end namespace

#endif
