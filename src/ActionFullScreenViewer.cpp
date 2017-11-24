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

#include <ActionFullScreenViewer.h>
#include <KeyPressHandler.h>    // QTools
using FaceTools::ActionFullScreenViewer;
typedef FaceTools::InteractiveModelViewer IMV;

class ActionFullScreenViewer::KeyPressHandler : public QTools::KeyPressHandler
{
public:
    KeyPressHandler( ActionFullScreenViewer* afsv) : _afsv(afsv) {}

protected:
    virtual bool handleKeyPress( QKeyEvent* e)
    {
        bool handled = false;
        if ( e->type() == QEvent::KeyPress && e->matches( QKeySequence::FullScreen))
        {
            _afsv->toggleFullScreen();
            handled = true;
        }   // end if
        return handled;
    }   // end handleKeyPress

private:
    ActionFullScreenViewer* _afsv;
};  // end class


// public
ActionFullScreenViewer::ActionFullScreenViewer( IMV* viewer, const std::string& dname)
    : _viewer( viewer),
      _kphandler( NULL),
      _action( dname.c_str(), NULL),
      _isfull(false)
{
    _kphandler = new KeyPressHandler( this);
    _viewer->addKeyPressHandler(_kphandler);    // Required so the user can undo full screen
    _action.setShortcut( QKeySequence::FullScreen);
    _action.setCheckable(true);
    _action.setChecked(false);
    connect( &_action, &QAction::triggered, this, &ActionFullScreenViewer::toggleFullScreen);
}   // end ctor


ActionFullScreenViewer::~ActionFullScreenViewer()
{
    _viewer->removeKeyPressHandler(_kphandler);
    delete _kphandler;
}   // end dtor


// public slot
void ActionFullScreenViewer::toggleFullScreen()
{
    _isfull = !_isfull;
    _viewer->setFullScreen( _isfull);
    _action.setChecked(_isfull);
}   // end toggleFullScreen

