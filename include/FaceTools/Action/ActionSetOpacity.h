/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_SET_OPACITY_H
#define FACE_TOOLS_ACTION_SET_OPACITY_H

#include "FaceAction.h"
#include <QDoubleSpinBox>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSetOpacity : public FaceAction
{ Q_OBJECT
public:
    // Display name will be used for spin box widget's tool tip.
    explicit ActionSetOpacity( const QString&);

    QString toolTip() const override { return "Change the opacity of model surfaces.";}

    QWidget* widget() const override { return _opacitySpinBox;}

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;
    void purge( const FM*) override;

private slots:
    void _doOnValueChanged();

private:
    QDoubleSpinBox *_opacitySpinBox;
    void _setViewOpacity( Vis::FV*);
    void _updateOpacities( const FMV*);
    std::unordered_map<const Vis::FV*, float> _olaps;
    std::unordered_map<const Vis::FV*, float> _vbnds;
};  // end class

}}   // end namespaces

#endif
