/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACETOOLS_WIDGET_MESH_INFO_DIALOG_H
#define FACETOOLS_WIDGET_MESH_INFO_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QDialog>
#include <QValidator>

namespace Ui { class MeshInfoDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT MeshInfoDialog : public QDialog
{ Q_OBJECT
public:
    explicit MeshInfoDialog( QWidget *parent=nullptr);
    ~MeshInfoDialog() override;

    void set( const FM*);
    const FM* get() const { return _model;}

signals:
    void selectedManifold( int);

private slots:
    void doOnManifoldIndexChanged( int);

private:
    Ui::MeshInfoDialog *_ui;
    const FM *_model;
    const QString _dialogRootTitle;
    void reset();
};  // end class

}}   // end namespace

#endif
