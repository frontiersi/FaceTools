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

#ifndef FACE_TOOLS_ACTION_EXPORT_PDF_H
#define FACE_TOOLS_ACTION_EXPORT_PDF_H

#include "FaceAction.h"
#include <BaseReportTemplate.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    // Initialise PDF export with the locations of pdflatex and IDTVis::FVonverter.
    static bool init( const std::string& pdflatex, const std::string& idtfConverter);

    // Returns whether this action is available to use.
    static bool isAvailable();

    ActionExportPDF( Report::BaseReportTemplate*, const QIcon& icon=QIcon(), const QString& email="",
                     QWidget* parent=nullptr, QProgressBar* pb=nullptr);  // Is async if pb not null
    ~ActionExportPDF() override { delete _template;}

    // Set the path to the logo resouce.
    void setLogoResource( const QString& logo) { _logoFile = logo;}

    // Set author info to be embedded in all reports.
    void setAuthorInfo( const QString& ainfo) { _author = ainfo;}

    QWidget* getWidget() const override { return _template->getWidget();}

protected slots:
    bool testReady( const Vis::FV*) override;
    bool testEnabled( const QPoint* mc=nullptr) const override;
    bool doBeforeAction( FVS&, const QPoint&) override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet&, const FVS&, bool) override;
    void purge( const FaceModel*) override;

private:
    Report::BaseReportTemplate *_template;
    QWidget *_parent;
    QString _logoFile;
    QString _author;

    const RFeatures::ObjModel *_cmodel;
    RFeatures::CameraParams _cam;
    QString _pdffile;
    QString _err;
    bool writeLaTeX( const RFeatures::ObjModel*,
                     const RFeatures::CameraParams&,
                     const QString&,
                     const QString&,
                     const QString&,
                     std::vector<RModelIO::LaTeXU3DInserter::Ptr>&);
};  // end class

}   // end namespace
}   // end namespace

#endif
