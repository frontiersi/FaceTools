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

#ifndef FACE_TOOLS_ACTION_MAKE_HALF_FACE_H
#define FACE_TOOLS_ACTION_MAKE_HALF_FACE_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionMakeHalfFace : public FaceAction
{ Q_OBJECT
public:
    ActionMakeHalfFace( const QString&, const QIcon&);

    QString toolTip() const override;
    QString whatsThis() const override;

    // Set the direction in which the half space part of the model should
    // be copied from. The point p specifies a point on the cutting plane.
    void setPlane( const cv::Vec3f& n, const cv::Vec3f& p=cv::Vec3f(0,0,0));

    // If set true, the landmarks are used to identify the point in the cutting
    // plane as being the mean landmark position - the direction set as n (in setPlane).
    void setUseLandmarksIfPossible( bool);

protected:
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    void doAfterAction( Event) override;

private:
    cv::Vec3f _n, _p;
    bool _useLmks;
};  // end class

}}   // end namespace

#endif
