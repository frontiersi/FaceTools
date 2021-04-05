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
#include <r3dio/LatexWriter.h>
#include <sol.hpp>

namespace FaceTools { namespace Report {

class FaceTools_EXPORT Report : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<Report>;
    using CPtr = std::shared_ptr<const Report>;
    static Ptr load( const QString& luascript);

    static void setLogoPath( const QString&);
    static void setHeaderAppName( const QString&);
    static void setVersionString( const QString&);
    static void setInkscape( const QString&);
    static void setDefaultPageDims( const QSize&);  // Millimetres

    const QString& name() const { return _name;}
    const QString& title() const { return _title;}
    bool isAvailable() const; // Returns true iff this report can be generated.

    // Add a custom Lua function for report delegates with luaName as the name of the
    // function used within the Lua report itself and the function referring to a C++
    // delegate defined by the client. The addLatexHeader/Document functions should
    // be used from within C++ code to place the latex.
    void addCustomLuaFn( const QString &luaName,
            const std::function<void( const QRectF&)>&);
    void addCustomLuaFn( const QString &luaName,
            const std::function<void( const QRectF&, const FM*)>&);
    // Add the given Latex to the header of the document.
    void addLatexHeader( const QString&);
    // Add the given Latex at the given position in the document body.
    void addLatexDocument( const QRectF&, const QString&, bool centre);

    // Sanitise the given string for Latex command characters and return
    // a standard string suitable for writing to a standard stream object.
    static std::string sanit( const QString&);

    // Set the content of the report by calling out to the corresponding
    // Lua delegate functions for this report. This must be called in the
    // GUI thread since the _addLatexGrowthCurvesChart requires the use
    // of the QChartView object which assumes it's in the GUI thread.
    // If false is returned, the error message is retrieved using errorMsg().
    bool setContent();

    // Generate report returning true on success. If false is
    // returned, the error message is retrieved using errorMsg().
    bool generate();

    const QString pdffile() const { return _pdffile;} // Returns path to last generated PDF.
    inline const QString &errorMsg() const { return _errMsg;}

private:
    static QString s_logoPath;
    static QString s_headerName;
    static QString s_versionStr;
    static QString s_inkscape;
    static QSize s_pageDims;

    QString _name;
    QString _title;
    bool _twoModels;
    QSize _pageDims;
    sol::state _lua;
    sol::function _isAvailable;
    sol::function _setContent;
    r3dio::LatexWriter *_ltxw;
    QString _pdffile;
    QString _errMsg;
    bool _validContent;

    void _addLatexScanInfo( const QRectF&, const FM*);
    void _addLatexNotes( const QRectF&, const FM*);
    bool _addLatexPhenotypicTraits( const QRectF&, const FM*, int, int);
    void _addLatexFigure( const QRectF&, const FM*, const std::string&);
    void _addLatexSelectedColourMapFigure( const QRectF&, const std::string&);
    void _addLatexSelectedColourMapLegend( const QRectF&);
    // mid is the metric ID and d is the dimension of the metric.
    // Set footnotemark to something higher than zero to use footnote mark
    // notation for the chart data source instead of the whole source text.
    void _addLatexChart( const QRectF&, const FM*, int mid, size_t d=0, int footnotemark=0);
    void _addLatexFootnoteSources( const QRectF&, const FM*, const sol::table&);

    static bool _usingSVG();
    r3dio::Box _pageBox( const QRectF&) const;
    r3dio::Point _pagePoint( const Vec2f&) const;
    std::string _writeModelBGImage( const QRectF&, const FM*, const r3d::Mesh&);
    std::string _writeModelBGImage( const QRectF&, const FM*);
    bool _writeLatex();
    Report();
    ~Report() override;
    Report( const Report&) = delete;
    void operator=( const Report&) = delete;
};  // end class

}}   // end namespace

#endif
