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

#ifndef FACE_TOOLS_METRIC_INTERLANDMARK_METRIC_CALCULATOR_TYPE_H
#define FACE_TOOLS_METRIC_INTERLANDMARK_METRIC_CALCULATOR_TYPE_H

/**
 * A new InterlandmarkMetricCalculatorType should be created for each
 * kind of interlandmark measurement needed.
 */

#include <MetricCalculatorTypeInterface.h>
#include <EuclideanDistanceVisualiser.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT InterlandmarkMetricCalculatorType : public MetricCalculatorTypeInterface
{ Q_OBJECT
public:
    InterlandmarkMetricCalculatorType();    // Required for setting as template type in MetricCalculatorTypeRegistry.

    int id() const override { return _id;}
    std::string category() const override { return "Interlandmark";}
    std::string params() const override;
    std::string name() const override { return _name;}
    std::string source() const override { return _source;}
    std::string ethnicities() const override { return _ethnicities;}
    Sex sex() const override { return _sex;}
    std::string description() const override { return _desc;}
    bool isBilateral() const override { return _bilat;}
    size_t dims() const override { return 1;}
    rlib::RSD::Ptr rsd( size_t) const override { return _rsd;}
    size_t numDecimals() const override { return _ndps;}
    Vis::MetricVisualiser* visualiser() override { return &_edv;}

    void setId( int id) override;
    void setName( const std::string&) override;
    void setDescription( const std::string&) override;
    void setSource( const std::string&) override;
    void setEthnicities( const std::string&) override;
    void setSex( Sex) override;
    void setNumDecimals( size_t) override;
    void setRSD( size_t, rlib::RSD::Ptr rsd) override;

    bool isAvailable( const FM*) const override;
    double measure( size_t, const FM*, FaceLateral) const override;

    MCTI::Ptr fromParams( const std::string& params) const override;

private:
    int _id;
    bool _bilat;
    std::string _name, _desc, _source, _ethnicities;
    Sex _sex;
    int _lA, _lB;
    Vis::EuclideanDistanceVisualiser _edv;
    size_t _ndps;
    rlib::RSD::Ptr _rsd;

    InterlandmarkMetricCalculatorType( int lmA, int lmB, bool);
};  // end class

}}   // end namespaces

#endif
