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

#include <ActionShowScanInfo.h>
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
}   // end ctor


void ActionShowScanInfo::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _dialog = new ScanInfoDialog(p);
    connect( _dialog, &ScanInfoDialog::onUpdated, [this](){ emit onEvent(Event::METADATA_CHANGE);});
    connect( _dialog, &ScanInfoDialog::onAssessmentChanged, [this](){ emit onEvent(Event::ASSESSMENT_CHANGE);});
    connect( _dialog, &ScanInfoDialog::onCopiedLandmarks, [this](){ emit onEvent(Event::LANDMARKS_CHANGE);});
}   // end postInit


ActionShowScanInfo::~ActionShowScanInfo()
{
    delete _dialog;
}   // end dtor


void ActionShowScanInfo::setThumbnailUpdater( ActionUpdateThumbnail* act)
{
    _tupdater = act;
    const QSize dims = _dialog->thumbDims();
    _tupdater->setThumbnailSize( dims.width(), dims.height());
    connect( act, &ActionUpdateThumbnail::updated, this, &ActionShowScanInfo::doOnUpdatedThumbnail);
}   // end setThumbnailUpdater


bool ActionShowScanInfo::checkState( Event e)
{
    if ( EventGroup(e).has(Event::CLOSED_MODEL) && !MS::selectedModel())
        _dialog->hide();
    return !_dialog->isHidden();
}   // end checkState


bool ActionShowScanInfo::checkEnable( Event)
{
    FM* fm = ModelSelector::selectedModel();
    _dialog->set(fm);
    if ( fm && _tupdater)
        doOnUpdatedThumbnail( fm, _tupdater->thumbnail(fm));
    return fm;
}   // end checkEnable


void ActionShowScanInfo::doOnUpdatedThumbnail( const FM* fm, const cv::Mat& img)
{
    if ( _dialog->get() == fm)
        _dialog->setThumbnail(img);
}   // end doOnUpdatedThumbnail


void ActionShowScanInfo::doAction( Event)
{
    _dialog->refresh();
    _dialog->show();
    _dialog->raise();
    _dialog->activateWindow();
}   // end doAction


void ActionShowScanInfo::doAfterAction( Event)
{
    MS::showStatus( "Showing Assessment Information", 5000);
}   // end doAfterAction
