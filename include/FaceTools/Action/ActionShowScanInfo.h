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

#ifndef FACE_TOOLS_ACTION_SHOW_SCAN_INFO_H
#define FACE_TOOLS_ACTION_SHOW_SCAN_INFO_H

#include "ActionUpdateThumbnail.h"
#include <FaceTools/Widget/ScanInfoDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowScanInfo : public FaceAction
{ Q_OBJECT
public:
    ActionShowScanInfo( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Show/edit metadata about the selected subject.";}
    QWidget* widget() const override { return _dialog;}

    void setThumbnailUpdater( ActionUpdateThumbnail*);

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;

private:
    Widget::ScanInfoDialog *_dialog;
};  // end class

}}   // end namespace

#endif
