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

#ifndef FACE_TOOLS_ACTION_RENAME_PATH_H
#define FACE_TOOLS_ACTION_RENAME_PATH_H

#include "FaceAction.h"
#include <QInputDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionRenamePath : public FaceAction
{ Q_OBJECT
public:
    ActionRenamePath( const QString&, const QIcon&, const QKeySequence &ks=QKeySequence());

    QString toolTip() const override { return "(Re)name the selected user measurement.";}

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private slots:
    void _doOnTextValueChanged( const QString&);

private:
    QInputDialog *_dialog;
    QString _label;
};  // end class

}}   // end namespace

#endif
