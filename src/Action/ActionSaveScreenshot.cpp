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

#include <Action/ActionSaveScreenshot.h>
#include <FaceModelViewer.h>
using FaceTools::Action::ActionSaveScreenshot;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using MS = FaceTools::ModelSelect;


ActionSaveScreenshot::ActionSaveScreenshot( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _fdialog(nullptr)
{
}   // end ctor


void ActionSaveScreenshot::postInit()
{
    _fdialog = new QFileDialog( static_cast<QWidget*>(parent()));
    _fdialog->setWindowTitle( tr("Save image as..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setViewMode( QFileDialog::Detail);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
    _fdialog->setNameFilter( tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp)"));
}   // end postInit


QString ActionSaveScreenshot::whatsThis() const
{
    QStringList htxt;
    htxt << "Save a snapshot of the model viewer region. If the adjacent viewers are visible,";
    htxt << "the snapshots from all of them are concatenated together horizontally before saving.";
    return htxt.join( " ");
}   // end whatsThis


namespace {
bool hasValidExtension( const QString& fname)
{
    return fname.endsWith( ".jpg", Qt::CaseInsensitive)
        || fname.endsWith( ".jpeg", Qt::CaseInsensitive)
        || fname.endsWith( ".png", Qt::CaseInsensitive)
        || fname.endsWith( ".gif", Qt::CaseInsensitive)
        || fname.endsWith( ".bmp", Qt::CaseInsensitive);
}   // end hasValidExtension
}   // end namespace


bool ActionSaveScreenshot::isAllowed( Event) { return MS::isViewSelected();}


void ActionSaveScreenshot::doAction( Event)
{
    QString fname;
    if ( _fdialog->exec())
        fname = _fdialog->selectedFiles().first().trimmed();

    if ( !fname.isEmpty())
    {
        if ( !hasValidExtension(fname)) // So that OpenCV doesn't have a fit.
            fname += ".jpg";

        std::vector<cv::Mat> imgs;
        for ( FMV* fmv : MS::viewers())
        {
            QSize sz = fmv->size();
            if ( fmv->isVisible() && sz.width() > 0 && sz.height() > 0)
                imgs.push_back( fmv->grabImage());
        }   // end for
        cv::Mat img = r3d::concatHorizontalMax( imgs);
        cv::imwrite( fname.toLocal8Bit().toStdString(), img);
    }   // end if
}   // end doAction
