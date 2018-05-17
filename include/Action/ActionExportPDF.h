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
#include <boost/filesystem/path.hpp>

namespace FaceApp {
namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    ActionExportPDF( Report::BaseReportTemplate*, QWidget*, QProgressBar* pb=NULL);  // Is async if pb not NULL

    QWidget* getWidget() const override { return _template->getWidget();}

protected slots:
    bool testReady( FaceControl*) override;
    bool testEnabled() override;
    bool doBeforeAction( FaceControlSet&) override;
    bool doAction( FaceControlSet&) override;
    void doAfterAction( const FaceControlSet&, bool) override;

private:
    Report::BaseReportTemplate *_template;
    QWidget *_parent;

    boost::filesystem::path _workdir;
    boost::filesystem::path _logopath;
    std::string _pdffile;
    std::string _err;
};  // end class

}   // end namespace
}   // end namespace

#endif
