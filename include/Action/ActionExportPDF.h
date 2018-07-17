/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
    // Initialise PDF export with the locations of pdflatex and IDTFConverter.
    static bool init( const std::string& pdflatex, const std::string& idtfConverter);

    ActionExportPDF( Report::BaseReportTemplate*, const QIcon& icon=QIcon(),
                     QWidget* parent=nullptr, QProgressBar* pb=nullptr);  // Is async if pb not null
    ~ActionExportPDF() override { delete _template;}

    // Set the path to the logo resouce.
    void setLogoResource( const QString& logo) { _logoFile = logo;}

    // Set author info to be embedded in all reports.
    void setAuthorInfo( const QString& ainfo) { _author = ainfo;}

    QWidget* getWidget() const override { return _template->getWidget();}

protected slots:
    bool testReady( const FaceControl*) override;
    bool testEnabled() const override;
    bool doBeforeAction( FaceControlSet&) override;
    bool doAction( FaceControlSet&) override;
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;
    void purge( const FaceModel*) override;

private:
    Report::BaseReportTemplate *_template;
    QWidget *_parent;
    QString _logoFile;
    QString _author;

    std::string _pdffile;
    std::string _err;
    bool writeLaTeX( const FaceModel*,
                     const RFeatures::CameraParams&,
                     const std::string&,
                     const std::string&,
                     const std::string&,
                     std::vector<RModelIO::LaTeXU3DInserter::Ptr>&);
};  // end class

}   // end namespace
}   // end namespace

#endif
