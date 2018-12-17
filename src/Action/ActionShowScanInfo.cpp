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
#include <FaceView.h>
#include <cmath>
#include <cassert>
using FaceTools::Action::ActionShowScanInfo;
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventSet;
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


ActionShowScanInfo::ActionShowScanInfo( const QString& dname, const QIcon& icon, QWidget* parent)
    : FaceAction( dname, icon), _dialog( new ScanInfoDialog(parent)), _tupdater(nullptr)
{
    connect( _dialog, &ScanInfoDialog::onUpdated, this, &ActionShowScanInfo::doOnUpdated);
    setAsDialogShower( _dialog);
}   // end ctor


ActionShowScanInfo::~ActionShowScanInfo()
{
    delete _dialog;
}   // end dtor


void ActionShowScanInfo::setThumbnailUpdater( ActionUpdateThumbnail* act)
{
    _tupdater = act;
    const int dims = _dialog->minThumbDims();
    _tupdater->setThumbnailSize( dims, dims);
    connect( act, &ActionUpdateThumbnail::updated, this, &ActionShowScanInfo::doOnUpdatedThumbnail);
}   // end setThumbnailUpdater


void ActionShowScanInfo::tellReady( const FV* fv, bool v)
{
    FM* fm = nullptr;
    if ( fv && v)
        fm = fv->data();
    _dialog->set( fm);
    if ( fm && _tupdater)
        doOnUpdatedThumbnail( fm, _tupdater->thumbnail(fm));
}   // end tellReady


void ActionShowScanInfo::doOnUpdated( FM* fm)
{
    EventSet eset;
    eset.insert(METADATA_CHANGE);
    FVS fvs;
    fvs.insert(fm);
    emit reportFinished( eset, fvs, true);
}   // end doOnUpdated


void ActionShowScanInfo::doOnUpdatedThumbnail( const FM* fm, const cv::Mat& img)
{
    if ( _dialog->get() == fm)
        _dialog->setThumbnail(img);
}   // end doOnUpdatedThumbnail
