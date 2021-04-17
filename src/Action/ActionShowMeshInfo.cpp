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

#include <Action/ActionShowMeshInfo.h>
using FaceTools::Action::ActionShowMeshInfo;
using FaceTools::Action::ActionDiscardManifold;
using FaceTools::Action::ActionRemoveManifolds;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::MeshInfoDialog;
using MS = FaceTools::ModelSelect;



ActionShowMeshInfo::ActionShowMeshInfo( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _dialog(nullptr), _dmact(nullptr), _rmact(nullptr)
{
    addRefreshEvent( Event::MESH_CHANGE | Event::MODEL_SELECT);
}   // end ctor


void ActionShowMeshInfo::setDiscardManifoldAction( ActionDiscardManifold *a)
{
    _dmact = a;
    if ( _dialog && _dmact)
        _dialog->setDiscardManifoldAction( _dmact);
}   // end setDiscardManifoldAction


void ActionShowMeshInfo::setRemoveManifoldsAction( ActionRemoveManifolds *a)
{
    _rmact = a;
    if ( _dialog && _rmact)
        _dialog->setRemoveManifoldsAction( _rmact);
}   // end setRemoveManifoldsAction


void ActionShowMeshInfo::postInit()
{
    _dialog = new MeshInfoDialog( static_cast<QWidget*>(parent()));
    connect( _dialog, &MeshInfoDialog::onSelectedManifoldChanged,
             this, &ActionShowMeshInfo::_doOnSelectedManifoldChanged);
    connect( _dialog, &MeshInfoDialog::finished,
             this, &ActionShowMeshInfo::_doOnDialogClosed);
    if ( _dmact)
        _dialog->setDiscardManifoldAction( _dmact);
    if ( _rmact)
        _dialog->setRemoveManifoldsAction( _rmact);
}   // end postInit


bool ActionShowMeshInfo::update( Event)
{
    if ( _dialog->isVisible())
    {
        if ( !MS::isViewSelected())
            _dialog->hide();
        else
            _dialog->refresh();
    }   // end if

    return _dialog->isVisible();
}   // end update


bool ActionShowMeshInfo::isAllowed( Event) { return MS::isViewSelected();}


void ActionShowMeshInfo::doAction( Event)
{
    _dialog->setVisible( true);
}   // end doAction


Event ActionShowMeshInfo::doAfterAction( Event)
{
    if (_dialog->isVisible())
        MS::showStatus( "Showing Model Information", 5000);
    return Event::NONE;
}   // end doAfterAction


void ActionShowMeshInfo::purge( const FM *fm)
{
    for ( Vis::FV *fv : fm->fvs())
        fv->purge( &_vis);
}   // end purge


void ActionShowMeshInfo::_doOnSelectedManifoldChanged( int midx)
{
    Vis::FV *fv = MS::selectedView();
    _vis.setManifoldIndex(midx);
    _dmact->setManifoldIndex(midx);
    _rmact->setManifoldIndex(midx);
    fv->apply( &_vis);
    fv->viewer()->updateRender();
}   // end _doOnSelectedManifoldChanged


void ActionShowMeshInfo::_doOnDialogClosed()
{
    _vis.purgeAll();
    MS::updateRender();
}   // end _doOnDialogClosed
