#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_H

/**
 * Understands how to perform and record a generic metric calculation.
 */

#include <MetricCalculatorTypeInterface.h>
#include <MetricSet.h>
#include <QtCharts/QChart>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricCalculator : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<MetricCalculator>;

    // Load from file. On error, return null.
    static Ptr fromFile( const std::string& filepath);
    static Ptr create( MCTI::Ptr);

    void setType( MCTI::Ptr mcti) { _mcti = mcti;}
    MCTI::Ptr type() const { return _mcti;}

    int id() const { return _mcti->id();}
    QString name() const { return _mcti->name().c_str();}
    QString category() const { return _mcti->category().c_str();}
    QString description() const { return _mcti->description().c_str();}
    QString ethnicities() const { return _mcti->ethnicities().c_str();}
    QString source() const { return _mcti->source().c_str();}
    Sex sex() const { return _mcti->sex();}
    size_t dims() const { return _mcti->dims();}
    size_t numDecimals() const { return _mcti->numDecimals();}
    bool isBilateral() const { return _mcti->isBilateral();}

    bool isAvailable( const FM* fm) const { return _mcti->isAvailable(fm);}
    Vis::MetricVisualiser *visualiser() const { return _mcti->visualiser();}

    // Whether or not this metric should be visible (true by default).
    void setVisible( bool v) { _visible = v && _mcti->visualiser() != nullptr;}
    bool isVisible() const { return _mcti && _mcti->visualiser() && _visible;}

    // Returns the age mapping interval range and sets out parameters if not null with min and max age values.
    double addSeriesToChart( QtCharts::QChart*, double *xmin=nullptr, double *xmax=nullptr) const;

    // Carry out the calculation, returning true on success.
    bool calculate( FM*) const;

    void signalUpdated() { emit updated();}  // Simply fire the updated signal.

signals:
    void updated();

private:
    MCTI::Ptr _mcti;
    bool _visible;

    MetricValue calcMetricValue( const FM* fm, FaceLateral) const;
    MetricCalculator();
    explicit MetricCalculator( MCTI::Ptr);
    ~MetricCalculator(){}
};  // end class

FaceTools_EXPORT std::ostream& operator<<( std::ostream&, const MetricCalculator&);
FaceTools_EXPORT std::istream& operator>>( std::istream&, MetricCalculator&);

}}   // end namespaces

#endif
