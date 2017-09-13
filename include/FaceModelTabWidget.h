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

#ifndef FACE_TOOLS_FACE_MODEL_TAB_WIDGET_H
#define FACE_TOOLS_FACE_MODEL_TAB_WIDGET_H

#include "FaceModelWidget.h"
#include <QTabWidget>

namespace FaceTools
{

class FaceTools_EXPORT FaceModelTabWidget : public QTabWidget
{ Q_OBJECT
public:
    FaceModelTabWidget( QMenu* contextMenu, const QList<QAction*>* actions, QWidget *parent=NULL);
    virtual ~FaceModelTabWidget();

signals:
    void onViewSelected( FaceModel*, const std::string&);    // Passes model and view title

public slots:
    int addTabWidget( FaceModel*);               // Add model as new tab returning tab index and making it active.
    size_t removeModel( FaceModel*);             // Remove model from all tabs returning number of views removed.
    size_t removeTabWidget( int);                // Remove the tab and all models in it returning the number of models removed.
    void addViewToTab( int, FaceModel*);         // Add a view to a tab returning tab.
    size_t removeModelFromTab( int, FaceModel*); // Remove model from tab returning num views removed.

    // Replace all the tabs in this tab group with a single tab containing
    // views of all the models in the tab group. Does nothing if no tabs yet.
    void consolidateTabs();

private slots:
    void showContextMenu( const QPoint&);
    void onTabChanged(int);

private:
    QMenu* _cmenu;
    const QList<QAction*>* _actions;
    InteractiveModelViewer *_viewer;
    QVBoxLayout *_viewerLayout;

    FaceModelTabWidget( const FaceModelTabWidget&); // No copy
    void operator=( const FaceModelTabWidget&);     // No copy
};  // end class

}   // end namespace

#endif

