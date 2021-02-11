/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <FaceTools/FaceTypes.h>
#include <FaceTools/FaceModel.h>
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
    void addCustomLuaFn( const QString&, const std::function<void( const FM*)>&);

    // Add some custom latex from within the generate function.
    void addCustomLatex( const QString&);

    void setHeaderName( const QString& n){ _headerName = n;}
    void setInkscape( const QString& exe){ _inkscape = exe;}    // Inkscape exe (for SVG output)

    const QString& name() const { return _name;}
    const QString& title() const { return _title;}

    // Return true iff this report can be generated.
    bool isAvailable() const;

    // Set the content of the report by calling out to the corresponding
    // Lua delegate functions for this report. This must be called in the
    // GUI thread since the _addLatexGrowthCurvesChart requires the use
    // of the QChartView object which assumes it's in the GUI thread.
    // If false is returned, the error message is retrieved using errorMsg().
    bool setContent();

    // Generate report returning true on success. If false is
    // returned, the error message is retrieved using errorMsg().
    bool generate();

    // Return the path to the generated PDF.
    QString pdffile() const;

    inline const QString &errorMsg() const { return _errMsg;}

private:
    QTemporaryDir& _tmpdir;
    QString _headerName;
    QString _inkscape;
    QString _name;
    QString _title;
    bool _twoModels;
    sol::state _lua;
    sol::function _isAvailable;
    sol::function _setContent;
    QTextStream *_os;
    QString _errMsg;
    bool _validContent;

    void _addLatexTestFigure( float wmm, float hmm);

    void _addLatexFigure( const FM*, float widthMM, float heightMM, const std::string& caption);

    // mid is the metric ID and d is the dimension of the metric.
    // Set footnotemark to something higher than zero to use footnote mark
    // notation for the chart data source instead of the whole source text.
    void _addLatexGrowthCurvesChart( const FM*, int mid, size_t d=0, int footnotemark=0);

    void _addLatexScanInfo( const FM*);
    void _addLatexNotes( const FM*);
    bool _addLatexPhenotypicTraits( const FM*, int, int);
    int _getNumPhenotypicTraits( const FM*) const;

    void _addLatexStartMinipage();
    void _addLatexEndMinipage();
    void _addLatexLineBreak();

    QString _metricCurrentSource( const FM*, int) const;
    std::unordered_map<int, int> _footnoteIndices( const FM*, const sol::table&) const;
    void _addFootnoteSources( const FM*, const sol::table&);

    bool _useSVG() const;
    void _writeLatex( QTextStream&);
    Report( const Report&) = delete;
    void operator=( const Report&) = delete;
};  // end class

}}   // end namespace

#endif
