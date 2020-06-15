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

#ifndef FACE_TOOLS_ACTION_SAVE_AS_FACE_MODEL_H
#define FACE_TOOLS_ACTION_SAVE_AS_FACE_MODEL_H

#include "FaceAction.h"
#include <QFileDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSaveAsFaceModel : public FaceAction
{ Q_OBJECT
public:
    ActionSaveAsFaceModel( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence::SaveAs);

    QString toolTip() const override { return "Save the selected model to a new file.";}

    QString whatsThis() const override;

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    QFileDialog *_fdialog;
    QString _filename;
    bool _discardTexture;
    Event _egrp;
};  // end class

}}   // end namespace

#endif
