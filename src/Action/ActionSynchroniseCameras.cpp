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

#include <Action/ActionSynchroniseCameras.h>
#include <Interactor/LandmarksHandler.h>
#include <Interactor/PathsHandler.h>
#include <Interactor/ViewerNotifier.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSynchroniseCameras;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::ModelViewer;
using FaceTools::Interactor::ViewerNotifier;
using MS = FaceTools::ModelSelect;
using FaceTools::Vec3f;


namespace {
class CameraSyncher : public ViewerNotifier
{
public:
    CameraSyncher() : _issync(false), _ivwr(nullptr) {}

    void update()
    {
        cameraStart();
        if ( _ivwr)
            cameraMove();
        cameraStop();
    }   // end update

    bool isSynching() const { return _issync;}
    void setSynching( bool v) { _issync = v;}

protected:
    void cameraStart() override
    {
        _ivwr = MS::selectedViewer();
        _setPickingEnabled( false);
        if ( isSynching() && _ivwr)
            for ( ModelViewer* v : MS::viewers())
                if ( v != _ivwr)
                    _sset.push_back(v);
    }   // end cameraStart

    void cameraStop() override
    {
        _setPickingEnabled( true);
        _ivwr = nullptr;
        _sset.clear();
    }   // end cameraStop

    void cameraMove() override
    {
        if ( !_ivwr)
            return;

        const r3d::CameraParams cnow = _ivwr->camera();
        const double pscale = _ivwr->getRenderer()->GetActiveCamera()->GetParallelScale();
        for ( ModelViewer* v : _sset)
        {
            v->setCamera(cnow);
            v->getRenderer()->GetActiveCamera()->SetParallelScale( pscale);
            v->updateRender();
        }   // end for

        _ivwr->refreshClippingPlanes(); // Do this for zoom

        static const QString dmsg( "distance: %1 %2");
        static const QString fmsg( "focus: %1 X, %2 Y, %3 Z");
        static const QString pmsg( "direction: %1 X, %2 Y, %3 Z");
        const QString m0 = dmsg.arg(cnow.distance()).arg(FaceTools::FM::LENGTH_UNITS);
        const Vec3f &fv = cnow.focus();
        const QString m1 = fmsg.arg( fv[0], -1, 'f', 2).arg( fv[1], -1, 'f', 2).arg( fv[2], -1, 'f', 2);
        Vec3f pv = fv - cnow.pos();
        pv.normalize();
        const QString m2 = pmsg.arg( pv[0], -1, 'f', 2).arg( pv[1], -1, 'f', 2).arg( pv[2], -1, 'f', 2);
        MS::showStatus(QString("Camera %1, %2, %3").arg(m0).arg(m1).arg(m2), 10000);
    }   // end cameraMove

private:
    bool _issync;
    FMV* _ivwr;
    std::vector<ModelViewer*> _sset;

    static void _setPickingEnabled( bool v)
    {
        const FV *fv = MS::selectedView();
        assert(fv);
        using FaceTools::Interactor::LandmarksHandler;
        LandmarksHandler *lh = MS::handler<LandmarksHandler>();
        if ( lh)
            lh->visualisation().setPickable( fv, v);
        using FaceTools::Interactor::PathsHandler;
        PathsHandler *ph = MS::handler<PathsHandler>();
        if ( ph)
            ph->visualisation().setPickable( fv, v);
    }   // end _setPickingEnabled
};  // end class
}   // end namespace


// public
ActionSynchroniseCameras::ActionSynchroniseCameras( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _camSync(new CameraSyncher)
{
    setCheckable(true, false);
    _camSync->setEnabled(true);
    CameraSyncher *cs = static_cast<CameraSyncher*>(_camSync);
    cs->setSynching( true);
    addRefreshEvent( Event::CAMERA_CHANGE);
}   // end ctor


ActionSynchroniseCameras::~ActionSynchroniseCameras()
{
    delete _camSync;
    _camSync = nullptr;
}   // end dtor


bool ActionSynchroniseCameras::isAllowed(Event) { return true;/*MS::isViewSelected();*/}


bool ActionSynchroniseCameras::update( Event e)
{
    //if ( has( e, Event::CAMERA_CHANGE) && _camSync->isSynching())
    CameraSyncher *cs = static_cast<CameraSyncher*>(_camSync);
    if ( MS::isViewSelected())
        cs->update();
    return cs->isSynching();
}   // end update


void ActionSynchroniseCameras::doAction( Event)
{
    CameraSyncher *cs = static_cast<CameraSyncher*>(_camSync);
    cs->setSynching( isChecked());
    cs->update();
}   // end doAction
