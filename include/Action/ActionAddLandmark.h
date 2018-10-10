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

#ifndef FACE_TOOLS_ACTION_ADD_LANDMARK_H
#define FACE_TOOLS_ACTION_ADD_LANDMARK_H

#include "ActionEditLandmarks.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionAddLandmark : public FaceAction
{ Q_OBJECT
public:
    ActionAddLandmark( const QString& dname, const QIcon& icon, ActionEditLandmarks*, QWidget* parent=nullptr);

protected slots:
    bool testEnabled( const QPoint* mc=nullptr) const override;
    bool doBeforeAction( FVS&, const QPoint&) override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(LANDMARKS_ADD);}

private:
    ActionEditLandmarks *_editor;
    QWidget *_parent;
    std::string _nname;
};  // end class

}}   // end namespaces

#endif
