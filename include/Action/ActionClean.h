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

#ifndef FACE_TOOLS_ACTION_CLEAN_H
#define FACE_TOOLS_ACTION_CLEAN_H

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionClean : public FaceAction
{ Q_OBJECT
public:
    ActionClean( QWidget* parent, QProgressBar* pb=NULL);   // Is async if pb not NULL
    ~ActionClean() override;

    QString getDisplayName() const override { return "Clean Mesh";}
    const QIcon* getIcon() const override { return &_icon;}

protected slots:
    bool testReady( FaceControl*) override;
    bool doAction( FaceControlSet&) override;
    void doAfterAction( const FaceControlSet&, bool) override;
    void respondToChange( FaceControl*) override;
    void burn( const FaceControl*) override;

private:
    QIcon _icon;
    QWidget *_parent;
    std::unordered_map<const FaceModel*, bool> _clean;
    std::unordered_set<const FaceModel*> _failset;
    bool checkClean( const FaceModel*);
    void showCleaningError() const;
};  // end class

}   // end namespace
}   // end namespace

#endif
