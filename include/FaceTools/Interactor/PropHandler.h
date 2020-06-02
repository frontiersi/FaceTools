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

#ifndef FACE_TOOLS_PROP_HANDLER_H
#define FACE_TOOLS_PROP_HANDLER_H

#include "MouseHandler.h"

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT PropHandler : public QObject, public MouseHandler
{ Q_OBJECT
public:
    PropHandler();

    virtual void refreshState(){}

protected slots:
    virtual void doEnterProp( Vis::FV*, const vtkProp*){}
    virtual void doLeaveProp( Vis::FV*, const vtkProp*){}
};  // end class

}}   // end namespaces

#endif
