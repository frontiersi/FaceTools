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

#ifndef FACE_TOOLS_ACTION_ACTION_SHOW_PHENOTYPES_H
#define FACE_TOOLS_ACTION_ACTION_SHOW_PHENOTYPES_H

#include "FaceAction.h"
#include <FaceTools/Widget/PhenotypesDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowPhenotypes : public FaceAction
{ Q_OBJECT
public:
    ActionShowPhenotypes( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Display the HPO terms browser.";}
    QWidget* widget() const override { return _dialog;}

protected:
    void postInit() override;
    bool update( Event) override;
    void doAction( Event) override;

private:
    Widget::PhenotypesDialog *_dialog;
};  // end class

}}   // end namespaces

#endif
