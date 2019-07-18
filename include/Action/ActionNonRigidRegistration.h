/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_NON_RIGID_REGISTRATION_H
#define FACE_TOOLS_ACTION_NON_RIGID_REGISTRATION_H

#include "FaceAction.h"
#include <QReadWriteLock>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionNonRigidRegistration : public FaceAction
{ Q_OBJECT
public:
    ActionNonRigidRegistration( const QString&, const QIcon&);

    QString toolTip() const override { return "Perform a non-rigid registration of the currently set template mask against the selected model.";}

    // Set the file path to the mask to perform coregistration with.
    // If the provided path is different to what is currently set, the
    // model is loaded. Loading is performed in a background thread.
    // Returns true if the provided filepath points to a valid model.
    static bool setMaskPath( const QString&);

    // Returns true iff the mask is loaded. Returns false if the model is
    // currently in the middle of being loaded.
    static bool maskLoaded();

    // Returns the currently set mask path.
    static QString maskPath();

    // Set the number of nearest neighbours to search for on target
    // models when performing correspondence with the template mask.
    // Initial/min value is 1, max value is 99.
    static void setKNN( int k);

protected:
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    void doAfterAction( Event) override;

private:
    static QString s_maskPath;
    static RFeatures::ObjModel::Ptr s_mask;
    static QReadWriteLock s_lock;
    static int s_knn;
};  // end class

}}   // end namespaces

#endif
