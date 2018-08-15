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

#ifndef FACE_TOOLS_ACTION_VISUALISE_H
#define FACE_TOOLS_ACTION_VISUALISE_H

#include "FaceAction.h"
#include <BaseVisualisation.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionVisualise : public FaceAction
{ Q_OBJECT
public:
    explicit ActionVisualise( Vis::BaseVisualisation*);

    QWidget* getWidget() const override { return _vis->getWidget();}

    bool isExclusive() const { return _vis->isExclusive();}

    Vis::BaseVisualisation* visualisation() { return _vis;}

    // Return true (default) if the visualisation manager should know about this visualisation.
    virtual bool manageVisualisation() const { return true;}

protected slots:
    bool testReady( const FaceControl*) override;
    void tellReady( FaceControl*, bool) override;   // Called whenever ready status changes
    bool testEnabled( const QPoint*) const override;
    bool testChecked( const FaceControl *fc=nullptr) const override;
    bool doAction( FaceControlSet&, const QPoint&) override;
    void doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool) override { cs.insert(VIEW_CHANGE);}
    bool displayDebugStatusProgression() const override { return false;}
    void purge( const FaceModel*) override;
    void clean( const FaceModel*) override;

private:
    void toggleVis( FaceControl*, const QPoint*);
    Vis::BaseVisualisation *_vis; // The visualisation delegate
};  // end class

}   // end namespace
}   // end namespace

#endif
