/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_SAVE_FACE_MODEL_H
#define FACE_TOOLS_ACTION_SAVE_FACE_MODEL_H

#include "ActionSaveAsFaceModel.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSaveFaceModel : public FaceAction
{ Q_OBJECT
public:
    ActionSaveFaceModel( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence::Save);

    void setSaveAsAction( ActionSaveAsFaceModel *sa) { _saveAs = sa;}

    QString toolTip() const override { return "Save the selected model.";}

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    std::unordered_map<QString, QStringList> _fails;    // Error messages --> filenames
    Event _egrp;
    ActionSaveAsFaceModel *_saveAs;
};  // end class

}}   // end namespace

#endif
