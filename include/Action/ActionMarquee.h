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

#ifndef FACE_TOOLS_ACTION_MARQUEE_H
#define FACE_TOOLS_ACTION_MARQUEE_H

#include "FaceAction.h"
#include "CameraWorker.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionMarquee : public FaceAction
{ Q_OBJECT
public:
    ActionMarquee( const QString& dname="Marquee (Demo)", const QIcon& icon=QIcon());
    ~ActionMarquee() override;

    void addViewer( FaceModelViewer*);  // Add a viewer that marquee mode will be applied to

protected slots:
    bool testEnabled() override;
    bool doAction( FaceControlSet&) override;

private:
    std::unordered_set<CameraWorker*> _workers;
    void stop();
};  // end class

}   // end namespace
}   // end namespace

#endif
