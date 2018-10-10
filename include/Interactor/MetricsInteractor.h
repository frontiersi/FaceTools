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

#ifndef FACE_TOOLS_INTERACTOR_METRICS_INTERACTOR_H
#define FACE_TOOLS_INTERACTOR_METRICS_INTERACTOR_H

#include "ModelEntryExitInteractor.h"

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT MetricsInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit MetricsInteractor( const MEEI*);

signals:
    void onEnterMetric( const Vis::FV*, int) const;
    void onLeaveMetric( const Vis::FV*, int) const;

private slots:
    void doOnEnterProp( const Vis::FV*, const vtkProp*);
    void doOnLeaveProp( const Vis::FV*, const vtkProp*);
};  // end class

}   // end namespace
}   // end namespace

#endif
