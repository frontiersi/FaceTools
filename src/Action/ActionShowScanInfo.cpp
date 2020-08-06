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

#include <Action/ActionShowScanInfo.h>
#include <FaceModel.h>
using FaceTools::Action::ActionShowScanInfo;
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionShowScanInfo::ActionShowScanInfo( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _tupdater(nullptr), _dialog(nullptr)
{
    setCheckable( true, false);
    addTriggerEvent( Event::MASK_CHANGE);
}   // end ctor


void ActionShowScanInfo::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _dialog = new ScanInfoDialog(p);
    connect( _dialog, &ScanInfoDialog::onAssessmentChanged, [this](){ emit onEvent(Event::ASSESSMENT_CHANGE);});
    connect( _dialog, &ScanInfoDialog::finished, [this](){ setChecked(false);});
}   // end postInit


void ActionShowScanInfo::setThumbnailUpdater( ActionUpdateThumbnail* act)
{
    _tupdater = act;
    const QSize dims = _dialog->thumbDims();
    _tupdater->setThumbnailSize( dims.width(), dims.height());
    connect( act, &ActionUpdateThumbnail::updated, this, &ActionShowScanInfo::doOnUpdatedThumbnail);
}   // end setThumbnailUpdater


bool ActionShowScanInfo::checkState( Event e)
{
    FM *fm = MS::selectedModel();
    if ( !fm || has( e, Event::CLOSED_MODEL))
        _dialog->hide();
    if ( _dialog->isVisible() && has( e, Event::METRICS_CHANGE | Event::MODEL_SELECT))
        _dialog->refresh();
    // Set check if a change of AM occurred with the setting of landmarks and the date of birth is the same
    // as the capture date. This equivalency ensures that this this dialog appears even for old files.
    return _dialog->isVisible() || (has( e, Event::MASK_CHANGE) && fm->hasLandmarks() && fm->dateOfBirth() == fm->captureDate());
}   // end checkState


bool ActionShowScanInfo::isAllowed( Event) { return MS::isViewSelected();}


void ActionShowScanInfo::doOnUpdatedThumbnail( const FM* fm, const cv::Mat& img) { _dialog->setThumbnail(img);}


void ActionShowScanInfo::doAction( Event)
{
    if ( isChecked())
    {
        FM *fm = MS::selectedModel();
        assert(fm);
        _dialog->refresh();
        if ( _tupdater)
            _dialog->setThumbnail( _tupdater->thumbnail(fm));
        _dialog->show();
        _dialog->raise();
        _dialog->activateWindow();
    }   // end if
}   // end doAction


Event ActionShowScanInfo::doAfterAction( Event) { return Event::NONE;}
