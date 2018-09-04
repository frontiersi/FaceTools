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

#include <VisualisationsManager.h>
#include <FaceActionManager.h>
using FaceTools::Action::VisualisationsManager;
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;


namespace FaceTools {
namespace Action {

class ActionShowTexture : public ActionVisualise
{
public:
    ActionShowTexture() : ActionVisualise( _vis = new Vis::TextureVisualisation, true/*show on load*/) {}
    ~ActionShowTexture() override { delete _vis;}
private:
    Vis::BaseVisualisation* _vis;
};  // end class

}   // end namespace
}   // end namespace


VisualisationsManager::VisualisationsManager()
    : QObject(), _evis(this), _nvis(this), _vact( new ActionShowTexture)
{
    _nvis.setExclusive(false);
}   // end ctor


void VisualisationsManager::init( FaceActionManager* fam)
{
    fam->addAction( _vact);
}   // end init


void VisualisationsManager::add( FaceAction* a)
{
    ActionVisualise* av = qobject_cast<ActionVisualise*>(a);
    if ( !av || !av->isVisible())
        return;

    if ( !av->visualisation()->isToggled())
        _evis.addAction(av->qaction());
    else
        _nvis.addAction(av->qaction());
}   // end add


QList<QAction*> VisualisationsManager::actions() const
{
    QList<QAction*> lst = _evis.actions();
    if ( !_nvis.actions().empty())
    {
        if ( !lst.isEmpty())
        {
            QAction* separator = new QAction;
            separator->setSeparator(true);
            lst.append( separator);
        }   // end if
        lst.append( _nvis.actions());
    }   // end if
    return lst;
}   // end actions
