#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_H

/**
 * Understands how to perform and record a generic metric calculation.
 */

#include <MetricCalculatorTypeInterface.h>
#include <GrowthData.h>
#include <MetricSet.h>
#include <Landmark.h>
#include <QtCharts/QChart>

namespace FaceTools { namespace Metric {

// Growth data by ethnicity, then by sex.
using MetricGrowthData = std::unordered_map<QString, std::unordered_map<int8_t, GrowthData*> >;

class FaceTools_EXPORT MetricCalculator : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<MetricCalculator>;

    // Load from file. On error, return null.
    static Ptr load( const QString& filepath);

    void setType( MCT* mct) { _mct = mct;}
    MCT* type() const { return _mct;}

    int id() const { return _id;}
    const QString& name() const { return _name;}
    const QString& description() const { return _desc;}
    size_t numDecimals() const { return _ndps;}
    QString category() const { return _mct->category();}
    size_t dims() const { return _dims;}
    bool isBilateral() const { return !_lmks1.empty();}

    Vis::MetricVisualiser *visualiser() const { return _mct->visualiser();}

    // Whether or not this metric should be visible (true by default).
    void setVisible( bool v) { _visible = v && _mct->visualiser() != nullptr;}
    bool isVisible() const { return _mct && _mct->visualiser() && _visible;}

    // Returns the age mapping interval range and sets out parameters if not null with min and max age values.
    double addSeriesToChart( QtCharts::QChart*, const GrowthData*, double *xmin=nullptr, double *xmax=nullptr) const;

    void addGrowthData( GrowthData*);

    bool hasGrowthData() const { return !_gdata.empty();}

    const GrowthData* growthData( const QString& ethnicity, int8_t sex) const;

    // Returns the growth data that best match the model's sex and ethnicity or null if none available.
    const GrowthData* growthData( const FM*) const;

    // Set the source to use (only needed if multiple distribution sources defined for this metric).
    bool setSource( const QString&);
    const QString& source() const { return _csrc;}  // Returns currently set source for metric calculations.

    // Return all sources available for this metric.
    const QStringSet& sources() const { return _sources;}

    // Return all ethnicities defined for the current source.
    const QStringSet& ethnicities() const;

    bool canCalculate( const FM* fm) const;

    // Carry out the calculation against current source, returning true on success.
    bool calculate( FM*) const;

    void signalUpdated() { emit updated( id());}  // Simply fire the updated signal passing this metric's id.
    void setSelected() { emit selected( id());}    // Tell others that this metric is the currently active one.

signals:
    void updated( int);
    void selected( int);

private:
    MCT* _mct;
    bool _visible;
    int _id;
    size_t _ndps, _dims;
    QString _name, _desc;
    Landmark::LmkList _lmks0, _lmks1;

    std::unordered_map<QString, MetricGrowthData> _gdata;   // Keyed by source
    std::unordered_set<GrowthData*> _agd;                   // All added growth data
    std::unordered_map<QString, QStringSet> _ethnicities;   // Keyed by source
    QStringSet _sources;
    QString _csrc;  // The current source to use (first added if not changed)
    QString _deth;  // The default ethnicity (first added)

    MetricValue calcMetricValue( const FM* fm, const Landmark::LmkList&) const;
    MetricCalculator();
    ~MetricCalculator();
};  // end class

}}   // end namespaces

#endif
