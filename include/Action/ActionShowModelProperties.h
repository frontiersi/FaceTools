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

#ifndef FACE_TOOLS_ACTION_SHOW_MODEL_PROPERTIES_H
#define FACE_TOOLS_ACTION_SHOW_MODEL_PROPERTIES_H

#include "ActionDiscardManifold.h"
#include <ModelPropertiesDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowModelProperties : public FaceAction
{ Q_OBJECT
public:
    ActionShowModelProperties( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());
    ~ActionShowModelProperties() override;

    QString toolTip() const override { return "Show information about the selected model's 3D representation.";}

protected:
    void postInit() override;
    bool checkState( Event) override;
    bool checkEnable( Event) override;
    void doAction( Event) override;

private:
    Widget::ModelPropertiesDialog *_dialog;
};  // end class

}}   // end namespace

#endif
