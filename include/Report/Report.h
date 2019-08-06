/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_REPORT_REPORT_H
#define FACE_TOOLS_REPORT_REPORT_H

#include <FaceTypes.h>
#include <QTemporaryDir>
#include <QTextStream>
#include <sol.hpp>

namespace FaceTools { namespace Report {

class FaceTools_EXPORT Report : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<Report>;
    using CPtr = std::shared_ptr<const Report>;
    static Ptr load( const QString& luascript, QTemporaryDir&);

    explicit Report( QTemporaryDir&);
    ~Report() override;

    // Add a custom Lua function for report delegates.
    void addCustomLuaFn( const QString&, const std::function<void()>&);

    // Add some custom latex from within the generate function.
    void addCustomLatex( const QString&);

    void setLogo( const QString& f) { _logofile = f;}
    void setHeaderName( const QString& n){ _headerName = n;}
    void setInkscape( const QString& exe){ _inkscape = exe;}    // Set the Inkscape exe (required for SVG output)

    const QString& name() const { return _name;}
    const QString& title() const { return _title;}

    // Return true iff this report can be generated for the given model.
    bool isAvailable( const FM*) const;

    // Generate a report for the given model and save to given filename returning true on success.
    // This function may take some time to run so best to run in separate thread and wait
    // for onFinishedGenerate signal to be emitted.
    bool generate( const FM*, const QString& u3dfile, const QString& pdffile);

signals:
    // Signal that report for the given model has finished being generated and is at the given location.
    // If parameter model pointer is null, the report failed to generate.
    void onFinishedGenerate( const FM*, const QString&);

private:
    QTemporaryDir& _tmpdir;
    QString _logofile, _headerName;
    QString _inkscape;
    QString _name;
    QString _title;
    sol::state _lua;
    sol::function _available;
    sol::function _addContent;

    QTextStream *_os;
    const FM* _model;
    QString _u3dfile;

    void _addLatexFigure( float widthMM, float heightMM, const std::string& caption);

    // mid is the metric ID and d is the dimension of the metric.
    // Set footnotemark to something higher than zero to use footnote mark
    // notation for the chart data source instead of the whole source text.
    void _addLatexGrowthCurvesChart( int mid, size_t d=0, int footnotemark=0);

    void _addLatexScanInfo();
    void _addLatexNotes();
    void _addLatexPhenotypicVariationsList();

    void _addLatexStartMinipage();
    void _addLatexEndMinipage();
    void _addLatexLineBreak();
    QString _metricCurrentSource(int) const;
    std::unordered_map<int, int> _footnoteIndices( const sol::table&) const;
    void _addFootnoteSources( const sol::table&);

    bool _useSVG() const;
    bool _writeLatex( QTextStream&) const;
    Report( const Report&) = delete;
    void operator=( const Report&) = delete;
};  // end class

}}   // end namespace

#endif
