/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
    ActionVisualise( Vis::BaseVisualisation*, bool visualiseOnLoad=false);

    QWidget* getWidget() const override { return _vis->getWidget();}

    Vis::BaseVisualisation* visualisation() { return _vis;}

protected slots:
    bool testReady( const Vis::FV*) override;
    void tellReady( Vis::FV*, bool) override;   // Called whenever ready status changes
    bool testEnabled( const QPoint*) const override;
    bool testIfCheck( const Vis::FV *fc=nullptr) const override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(VIEW_CHANGE);}
    bool displayDebugStatusProgression() const override { return false;}
    void purge( const FM*) override;
    void clean( const FM*) override;

private:
    bool toggleVis( Vis::FV*, const QPoint*);
    Vis::BaseVisualisation *_vis; // The visualisation delegate

    bool isVisAvailable( const FM*) const;
    bool isVisAvailable( const Vis::FV*, const QPoint* p=nullptr) const;
    bool isVisAvailable( const FVS&, const QPoint* p=nullptr) const;
    size_t setViewsToProcess( FVS&) const;
};  // end class

}   // end namespace
}   // end namespace

#endif
