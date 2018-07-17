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

#ifndef FACE_TOOLS_REPORT_SAMPLE_REPORT_H
#define FACE_TOOLS_REPORT_SAMPLE_REPORT_H

#include "BaseReportTemplate.h"

namespace FaceTools {
namespace Report {

class FaceTools_EXPORT SampleReport : public BaseReportTemplate
{ Q_OBJECT
public:
    SampleReport( const QString& dname) : BaseReportTemplate(dname) {}

    std::string reportTitle() const override { return "Sample Report";}
    size_t figureCount() const override { return 1;}

    RModelIO::LaTeXU3DInserter::Ptr createFigure( size_t idx, const std::string&,
                                                  const FaceModel*, const RFeatures::CameraParams&) override;

private:
};  // end class

}   // end namespace
}   // end namespace

#endif
