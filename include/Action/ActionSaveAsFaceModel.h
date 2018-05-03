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

#ifndef FACE_TOOLS_ACTION_SAVE_AS_FACE_MODEL_H
#define FACE_TOOLS_ACTION_SAVE_AS_FACE_MODEL_H

#include "FaceAction.h"
#include <FaceModelManager.h>
#include <QWidget>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionSaveAsFaceModel : public FaceAction
{ Q_OBJECT
public:
    ActionSaveAsFaceModel( FileIO::FaceModelManager*, QWidget *parent=NULL);

    QString getDisplayName() const override { return "Save &As";}
    const QIcon* getIcon() const override { return &_icon;}

protected slots:
    bool testEnabled() override { return readySet().size() == 1;}    // Only enabled for a single selected FaceControl
    bool doBeforeAction( FaceControlSet&) override;
    bool doAction( FaceControlSet&) override;
    void doAfterAction( const FaceControlSet&, bool) override;

private:
    FileIO::FaceModelManager *_fmm;
    QWidget *_parent;
    QIcon _icon;
    std::string _filename;
};  // end class

}   // end namespace
}   // end namespace

#endif
