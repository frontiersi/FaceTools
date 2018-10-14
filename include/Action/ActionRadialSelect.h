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

#ifndef FACE_TOOLS_ACTION_RADIAL_SELECT_H
#define FACE_TOOLS_ACTION_RADIAL_SELECT_H

/**
 * A visualisation with an internally constructed interactor to adjust
 * the radially visualised area on a set of views for a FaceModel.
 */

#include "ActionVisualise.h"
#include <ObjModelRegionSelector.h> // RFeatures
#include <LoopSelectVisualisation.h>
#include <RadialSelectInteractor.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionRadialSelect : public ActionVisualise
{ Q_OBJECT
public:
    ActionRadialSelect( const QString& dname, const QIcon& icon, Interactor::MEEI*, QStatusBar*);
    ~ActionRadialSelect() override;

    Interactor::MVI* interactor() override { return _interactor;}   // Return the interactor.

    double radius( const FaceModel *fm) const;
    cv::Vec3f centre( const FaceModel *fm) const;
    void selectedFaces( const FaceModel* fm, IntSet& fs) const;

private slots:
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet&, const FVS&, bool) override;
    void purge( const FaceModel*) override;

    void doOnIncreaseRadius( const Vis::FV*);
    void doOnDecreaseRadius( const Vis::FV*);
    void doOnSetCentre( const Vis::FV*, const cv::Vec3f&);

private:
    Vis::LoopSelectVisualisation *_vis;
    Interactor::RadialSelectInteractor *_interactor;
    double _radius;
    std::unordered_map<const FaceModel*, RFeatures::ObjModelRegionSelector::Ptr> _rsels;
    static const double MIN_RADIUS;
    void setRadius( const FaceModel*, double);
    void updateVis( const FaceModel*);
};  // end class

}   // end namespace
}   // end namespace

#endif
