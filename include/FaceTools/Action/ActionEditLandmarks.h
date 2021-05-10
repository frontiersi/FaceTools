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

#ifndef FACE_TOOLS_ACTION_EDIT_LANDMARKS_H
#define FACE_TOOLS_ACTION_EDIT_LANDMARKS_H

#include "FaceAction.h"
#include <FaceTools/Widget/LandmarksDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionEditLandmarks : public FaceAction
{ Q_OBJECT
public:
    ActionEditLandmarks( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Open the landmarks browser to reposition landmarks.";}
    QWidget* widget() const override { return _dialog;}

    // Set these before registering with the FaceActionManager
    void setShowLandmarksAction( FaceAction *act) { _actShow = act;}
    void setMirrorLandmarksAction( FaceAction *act) { _actMirror = act;}
    void setShowLandmarkLabelsAction( FaceAction *act) { _actShowLabels = act;}

signals:
    void onShowLandmarkLabels( bool);
    void onMirrorLandmarks();
    void onRestoreLandmarks();

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;

private slots:
    void _doOnStartedDrag( int, FaceSide);
    void _doOnFinishedDrag( int, FaceSide);

private:
    Widget::LandmarksDialog *_dialog;
    int _key;
    Vec3f _initPos; // Initial position of landmark being dragged at start
    Event _ev;
    FaceAction *_actShow;
    FaceAction *_actMirror;
    FaceAction *_actShowLabels;
    void _closeDialog();
    void _openDialog();
};  // end class

}}   // end namespace

#endif
