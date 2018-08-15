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

#ifndef FACE_TOOLS_ACTION_DETECT_FACE_H
#define FACE_TOOLS_ACTION_DETECT_FACE_H

#include "FaceAction.h"

namespace FaceTools {
namespace Detect {
class FaceDetector;
}   // end namespace

namespace Action {

class FaceTools_EXPORT ActionDetectFace : public FaceAction
{ Q_OBJECT
public:
    ActionDetectFace( const QString& dname, const QIcon& icon,
                      const QString& haarCascadesModelDir,
                      const QString& faceShapeLandmarksDat,
                      QWidget *parent=NULL, QProgressBar* pb=NULL);
    ~ActionDetectFace() override;

protected slots:
    bool testEnabled( const QPoint* mc=nullptr) const override { return _detector && ready1();}
    bool doBeforeAction( FaceControlSet&, const QPoint&) override;   // Warn if overwriting
    bool doAction( FaceControlSet&, const QPoint&) override;
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;

private:
    QWidget *_parent;
    Detect::FaceDetector *_detector;
    FaceModelSet _failSet;
};  // end class

}   // end namespace
}   // end namespace

#endif
