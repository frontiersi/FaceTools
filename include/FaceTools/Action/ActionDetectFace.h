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

#ifndef FACE_TOOLS_ACTION_DETECT_FACE_H
#define FACE_TOOLS_ACTION_DETECT_FACE_H

#include "FaceAction.h"
#include <FaceTools/Widget/LandmarksCheckDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionDetectFace : public FaceAction
{ Q_OBJECT
public:
    ActionDetectFace( const QString&, const QIcon&);

    QString toolTip() const override { return "Register correspondence mask and place facial landmarks.";}

    // Returns an empty string on success, otherwise the nature of the error.
    // Set ulmks specifies the ids of the landmarks to update (if any).
    static std::string detect( FM*, const IntSet& ulmks=IntSet());

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;   // Warn if overwriting
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    Widget::LandmarksCheckDialog *_cdialog;
    std::string _err;
    IntSet _ulmks;
    Event _ev;
};  // end class

}}   // end namespace

#endif
