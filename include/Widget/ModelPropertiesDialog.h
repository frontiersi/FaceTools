/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACETOOLS_WIDGET_MODEL_PROPERTIES_DIALOG_H
#define FACETOOLS_WIDGET_MODEL_PROPERTIES_DIALOG_H

#include <FaceTypes.h>
#include <QDialog>
#include <QValidator>

namespace Ui { class ModelPropertiesDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ModelPropertiesDialog : public QDialog
{ Q_OBJECT
public:
    explicit ModelPropertiesDialog( QWidget *parent=nullptr);
    ~ModelPropertiesDialog() override;

    void set( const FM*);
    const FM* get() const { return _model;}

signals:
    void selectedManifold( int);

private slots:
    void doOnManifoldIndexChanged( int);

private:
    Ui::ModelPropertiesDialog *_ui;
    const FM *_model;
    const QString _dialogRootTitle;
    void reset();
};  // end class

}}   // end namespace

#endif
