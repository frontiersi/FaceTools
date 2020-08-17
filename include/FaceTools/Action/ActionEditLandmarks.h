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

#ifndef FACE_TOOLS_ACTION_EDIT_LANDMARKS_H
#define FACE_TOOLS_ACTION_EDIT_LANDMARKS_H

#include "FaceAction.h"
#include <FaceTools/Widget/LandmarksDialog.h>
#include <FaceTools/Interactor/LandmarksHandler.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionEditLandmarks : public FaceAction
{ Q_OBJECT
public:
    ActionEditLandmarks( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Open the landmarks dialog to manually change landmark positions.";}
    QWidget* widget() const override { return _dialog;}

    Interactor::LandmarksHandler::Ptr handler() { return _handler;}

    // Set these before registering with the FaceActionManager
    void setShowLandmarksAction( FaceAction *act) { _actShow = act;}
    void setAlignLandmarksAction( FaceAction *act) { _actAlign = act;}
    void setRestoreLandmarksAction( FaceAction *act) { _actRestore = act;}
    void setShowLandmarkLabelsAction( FaceAction *act) { _actShowLabels = act;}

signals:
    void onShowLandmarkLabels( bool);
    void onAlignLandmarks();
    void onRestoreLandmarks();

protected:
    void postInit() override;
    bool checkState( Event) override;
    void doAction( Event) override;

    void saveState( UndoState&) const override;
    void restoreState( const UndoState&) override;

private slots:
    void _doOnClosedDialog();
    void _doOnStartedDrag( int, FaceSide);
    void _doOnDoingDrag( int, FaceSide);

private:
    Interactor::LandmarksHandler::Ptr _handler;
    Widget::LandmarksDialog *_dialog;
    FaceAction *_actShow;
    FaceAction *_actAlign;
    FaceAction *_actRestore;
    FaceAction *_actShowLabels;
    int _lmid;
    FaceSide _lat;
    Event _egrp;
};  // end class

}}   // end namespace

#endif
