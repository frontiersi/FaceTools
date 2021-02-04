/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_SMOOTH_H
#define FACE_TOOLS_ACTION_SMOOTH_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSmooth : public FaceAction
{ Q_OBJECT
public:
    ActionSmooth( const QString&, const QIcon&);

    QString toolTip() const override { return "Smooth surface geometry to reduce curvature at vertices.";}

    static void setMaxCurvature( double c);
    static double maxCurvature() { return s_maxc;}  // Default is 0.9

    static void setMaxIterations( size_t i);
    static size_t maxIterations() { return s_maxi;} // Default is 10

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;
    void saveState( UndoState&) const override;
    void restoreState( const UndoState&) override;

private:
    static double s_maxc;
    static size_t s_maxi;
    Event _ev;
};  // end class

}}   // end namespaces

#endif
