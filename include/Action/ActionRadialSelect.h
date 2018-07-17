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

#ifndef FACE_TOOLS_ACTION_RADIAL_SELECT_H
#define FACE_TOOLS_ACTION_RADIAL_SELECT_H

/**
 * A visualisation with an internally constructed interactor to adjust
 * the radially visualised area on a set of views for a FaceModel.
 */

#include "ActionVisualise.h"
#include <RadialSelectVisualisation.h>
#include <RadialSelectInteractor.h>
#include <QStatusBar>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionRadialSelect : public ActionVisualise
{ Q_OBJECT
public:
    ActionRadialSelect( const QString& dname, const QIcon& icon, FEEI*, QStatusBar*);
    ~ActionRadialSelect() override;

    Interactor::MVI* interactor() override { return _interactor;}   // Return the interactor.

    double radius( const FaceControl *fc) const { return _vis->radius(fc->data());}
    cv::Vec3f centre( const FaceControl *fc) const { return _vis->centre(fc->data());}

private slots:
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;
    void doOnSetNewCentre( FaceControl*, const cv::Vec3f&);
    void doOnSetNewRadius( FaceControl*, double);

private:
    Vis::RadialSelectVisualisation *_vis;
    Interactor::RadialSelectInteractor *_interactor;
    QStatusBar *_sbar;
};  // end class

}   // end namespace
}   // end namespace

#endif
