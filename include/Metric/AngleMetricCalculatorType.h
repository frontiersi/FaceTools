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

#ifndef FACE_TOOLS_METRIC_ANGLE_METRIC_CALCULATOR_TYPE_H
#define FACE_TOOLS_METRIC_ANGLE_METRIC_CALCULATOR_TYPE_H

#include <MetricCalculatorTypeInterface.h>
#include <AngleVisualiser.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT AngleMetricCalculatorType : public MetricCalculatorTypeInterface
{ Q_OBJECT
public:
    AngleMetricCalculatorType();    // Required for setting as template type in MetricCalculatorTypeRegistry.

    int id() const override { return _id;}
    const QString& name() const override { return _name;}
    const QString& description() const override { return _desc;}
    size_t numDecimals() const override { return _ndps;}

    void setId( int id) override;
    void setName( const QString&) override;
    void setDescription( const QString&) override;
    void setNumDecimals( size_t) override;

    QString category() const override { return s_cat;}
    QString params() const override;
    size_t dims() const override { return 1;}
    bool isBilateral() const override { return _bilat;}

    Vis::MetricVisualiser* visualiser() override { return &_vis;}
    bool canCalculate( const FM*) const override;

    double measure( size_t, const FM*, FaceLateral) const override;

    MCTI::Ptr fromParams( const QString& params) const override;

private:
    static const QString s_cat;
    int _id;
    bool _bilat;
    QString _name, _desc;
    int _l0, _lc, _l1;
    Vis::AngleVisualiser _vis;
    size_t _ndps;

    AngleMetricCalculatorType( int, int, int, bool);
};  // end class

}}   // end namespaces

#endif
