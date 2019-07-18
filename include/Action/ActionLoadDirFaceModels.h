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

#ifndef FACE_TOOLS_ACTION_LOAD_DIR_FACE_MODELS_H
#define FACE_TOOLS_ACTION_LOAD_DIR_FACE_MODELS_H

#include "FaceAction.h"
#include <LoadFaceModelsHelper.h>
#include <QFileDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionLoadDirFaceModels : public FaceAction
{ Q_OBJECT
public:
    ActionLoadDirFaceModels( const QString&, const QIcon&);
    ~ActionLoadDirFaceModels() override;

    QString toolTip() const override { return "Load all models found in a specified directory.";}

protected:
    void postInit() override;
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    void doAfterAction( Event) override;

private:
    FileIO::LoadFaceModelsHelper *_loadHelper;
    QFileDialog *_dialog;
};  // end class

}}   // end namespace

#endif
