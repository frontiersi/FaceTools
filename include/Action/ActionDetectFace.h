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

#ifndef FACE_TOOLS_ACTION_DETECT_FACE_H
#define FACE_TOOLS_ACTION_DETECT_FACE_H

#include "FaceAction.h"
#include <DetectionCheckDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionDetectFace : public FaceAction
{ Q_OBJECT
public:
    ActionDetectFace( const QString& dname, const QIcon& icon, QWidget *parent=nullptr, QProgressBar* pb=nullptr);

    // Returns an empty string on success, otherwise the nature of the error.
    // If ulmks is null, all landmarks are updated, otherwise just the set pointed to.
    static std::string redetectLandmarks( FM*, const IntSet* ulmks=nullptr);

protected slots:
    bool testReady( const Vis::FV*) override;
    bool testEnabled( const QPoint*) const override;
    bool doBeforeAction( FVS&, const QPoint&) override;   // Warn if overwriting
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet&, const FVS&, bool) override;

private:
    QWidget *_parent;
    Widget::DetectionCheckDialog *_cdialog;
    std::string _err;
    IntSet _ulmks;
};  // end class

}}   // end namespace

#endif
