/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionShowScanInfo.h>
#include <FaceModel.h>
using FaceTools::Action::ActionShowScanInfo;
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionShowScanInfo::ActionShowScanInfo( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _dialog(nullptr)
{
    setCheckable( true, false);
    addRefreshEvent( Event::METRICS_CHANGE | Event::MODEL_SELECT | Event::SAVED_MODEL);
    addTriggerEvent( Event::MASK_CHANGE);
}   // end ctor


void ActionShowScanInfo::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    assert(p);
    _dialog = new ScanInfoDialog(p);
    connect( _dialog, &ScanInfoDialog::onInfoChanged, [this](){ emit onEvent(Event::METADATA_CHANGE);});
    connect( _dialog, &ScanInfoDialog::finished, [this](){ setChecked(false);});
}   // end postInit


void ActionShowScanInfo::setThumbnailUpdater( ActionUpdateThumbnail* act)
{
    act->setThumbnailSize( _dialog->thumbDims());
    connect( act, &ActionUpdateThumbnail::updated, [=]( const FM* fm){ _dialog->setThumbnail( fm->thumbnail());});
}   // end setThumbnailUpdater


bool ActionShowScanInfo::update( Event e)
{
    const FM *fm = MS::selectedModel();
    if ( !fm)
        _dialog->hide();
    else
    {
        if ( has( e, Event::METRICS_CHANGE))
            _dialog->refreshAssessment();
        else
        {
            _dialog->refresh();
            _dialog->setThumbnail( fm->thumbnail());
        }   // end else if
    }   // end else
    return _dialog->isVisible();
}   // end update


bool ActionShowScanInfo::isAllowed( Event) { return MS::isViewSelected();}

bool ActionShowScanInfo::doBeforeAction( Event e) { return MS::isViewSelected();}

void ActionShowScanInfo::doAction( Event e)
{
    const FM *fm = MS::selectedModel();
    // Set check if a change of AM occurred with the setting of landmarks and the date of birth is the same
    // as the capture date. This equivalency ensures that this this dialog appears even for old 3DF file versions.
    const bool chk = isChecked() || (any( triggerEvents(), e) && fm->hasLandmarks() && fm->dateOfBirth() == fm->captureDate());
    if ( chk)
        _dialog->refresh();
    _dialog->setVisible( chk);
}   // end doAction

