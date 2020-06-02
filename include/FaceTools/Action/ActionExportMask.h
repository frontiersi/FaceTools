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

#ifndef FACE_TOOLS_ACTION_EXPORT_MASK_H
#define FACE_TOOLS_ACTION_EXPORT_MASK_H

#include "FaceAction.h"
#include <QFileDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportMask : public FaceAction
{ Q_OBJECT
public:
    ActionExportMask( const QString&, const QIcon&, const QKeySequence &ks=QKeySequence());

    QString toolTip() const override { return "Export the selected model's mask.";}

    QString whatsThis() const override;

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    QFileDialog *_fdialog;
    std::string _filename;
    bool _savedOkay;
};  // end class

}}   // end namespace

#endif
