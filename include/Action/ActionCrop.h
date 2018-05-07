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

#ifndef FACE_TOOLS_ACTION_CROP_H
#define FACE_TOOLS_ACTION_CROP_H

#include <ActionVisualise.h>
#include <BoundaryVisualisation.h>
#include <RadialSelectInteractor.h>
#include <QStatusBar>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionCrop : public FaceAction
{ Q_OBJECT
public:
    explicit ActionCrop( QStatusBar*);
    ~ActionCrop() override;

    ActionVisualise* visualiser() { return _vact;}  // Return the visualisation action.

    QString getDisplayName() const override { return "Crop";}
    const QIcon* getIcon() const override { return &_icon;}

protected slots:
    bool testReady( FaceControl* fc) override;
    bool testEnabled() const { return readyCount() == 1;}   // Only enabled for single selections
    void tellReady( FaceControl*, bool) override;
    bool doAction( FaceControlSet&) override;

private:
    QIcon _icon;
    QStatusBar *_sbar;
    Vis::BoundaryVisualisation *_bvis;
    ActionVisualise *_vact;
    Interactor::RadialSelectInteractor *_interactor;
};  // end class

}   // end namespace
}   // end namespace

#endif
