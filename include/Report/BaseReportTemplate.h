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

#ifndef FACE_TOOLS_REPORT_BASE_REPORT_TEMPLATE_H
#define FACE_TOOLS_REPORT_BASE_REPORT_TEMPLATE_H

#include "ReportInterface.h"

namespace FaceTools {
namespace Action {
class ActionExportPDF
}   // end namespace

namespace Report {

// Derived classes should inherit BaseReportTemplate instead of ReportInterface
class FaceTools_EXPORT BaseReportTemplate : public ReportInterface
{ Q_OBJECT
public:
    BaseReportTemplate( const QString& displayName, const QIcon&);
    explicit BaseReportTemplate( const QString& displayName);
    virtual ~BaseReportTemplate();

    QString getDisplayName() const override { return _dname;}
    const QIcon* getIcon() const override { return _icon;}

    bool isAvailable( const FaceModel*) const override { return true;}

protected:
    // Called from within ActionExportPDF constructor to set self on this report template.
    // Derived types may call addRespondTo functions of the ActionExportPDF to expand the
    // set of ChangeEvents that this action causes isAvailable() to be called for.
    virtual void setAction( Action::ActionExportPDF*){}

    // Destroy any cached data relating to the given FaceControl.
    virtual void burn( const FaceControl*){}

    friend class Action::ActionExportPDF;

private:
    const QString _dname;           // Display name
    const QIcon *_icon;             // Display icon

    BaseReportTemplate( const BaseReportTemplate&);
    void operator=( const BaseReportTemplate&);
};  // end class

}   // end namespace
}   // end namespace

#endif
