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

#include <Report/Report.h>
#include <Metric/MetricManager.h>
#include <Metric/PhenotypeManager.h>
#include <Metric/StatsManager.h>
#include <Metric/Chart.h>
#include <Action/ActionOrientCamera.h>
#include <Action/ActionUpdateThumbnail.h>
#include <Vis/ColourVisualisation.h>
#include <Widget/ChartDialog.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <U3DCache.h>
#include <rlib/MathUtil.h>
#include <QFile>
#include <boost/filesystem.hpp>
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MetricSet;
using FaceTools::Report::Report;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using FaceTools::FM;
using SM = FaceTools::Metric::StatsManager;
using MM = FaceTools::Metric::MetricManager;
using MC = FaceTools::Metric::Metric;
using MS = FaceTools::ModelSelect;
using r3d::CameraParams;
using r3d::Mesh;
using r3dio::LatexWriter;
namespace BFS = boost::filesystem;


// static definitions
QString Report::s_logoPath;
QString Report::s_headerName;
QString Report::s_versionStr;
QString Report::s_inkscape;
QSize Report::s_pageDims( 210, 297);    // A4 portrait by default

void Report::setLogoPath( const QString &logoPath) { s_logoPath = logoPath;}
void Report::setHeaderAppName( const QString &hn) { s_headerName = hn;}
void Report::setVersionString( const QString &vs) { s_versionStr = vs;}
void Report::setInkscape( const QString &exe) { s_inkscape = exe;}
void Report::setDefaultPageDims( const QSize &pd) { s_pageDims = pd;}
bool Report::_usingSVG() { return !s_inkscape.isEmpty() && QFile::exists(s_inkscape);}

namespace {

int getNumPhenotypicTraits( const FM *fm)
{
    const IntSet pids = PhenotypeManager::discover(*fm, -1); // Use the current assessment id
    return int(pids.size());
}   // end getNumPhenotypicTraits


std::string getSelectedViewActiveColoursName()
{
    const FaceTools::Vis::FV *fv = MS::selectedView();
    QString nm;
    if ( fv->activeColours())
    {
        nm = fv->activeColours()->label();
        nm.replace("\n", " ");
    }   // end if
    return Report::sanit(nm);
}   // end getSelectedViewActiveColoursName


std::string metricCurrentSource( const FM *fm, int mid)
{
    SM::RPtr gd = SM::stats( mid, fm);
    if ( !gd)
        return "";
    QString src = gd->source();
    if ( !gd->note().isEmpty())
        src += " " + gd->note();
    return Report::sanit(src);
}   // end metricCurrentSource


std::unordered_map<int, int> footnoteIndices( const FM *fm, const sol::table& mids)
{
    std::unordered_map<std::string, int> refs;
    std::unordered_map<int, int> idxs;
    int idx = 1;
    for ( size_t i = 0; i < mids.size(); ++i)
    {
        const int mid = mids[i];
        const std::string src = metricCurrentSource( fm, mid);
        if ( src.empty())
            continue;

        if ( refs.count(src) > 0)
            idxs[mid] = refs.at(src);
        else
        {
            refs[src] = mid;
            idxs[mid] = idx++;
        }   // end else
    }   // end for
    return idxs;
}   // end footnoteIndices
}   // end namespace


std::string Report::sanit( const QString &s) { return LatexWriter::sanit(s.toStdString());}


// private
Report::Report() : _ltxw(nullptr)
{
    _lua.open_libraries( sol::lib::base);
    _lua.open_libraries( sol::lib::math);

    _lua.new_enum( "FaceSide",
                   "MID", FaceSide::MID,
                   "LEFT", FaceSide::LEFT,
                   "RIGHT", FaceSide::RIGHT);

    _lua.new_usertype<MetricSet>( "MetricSet",
                                  "metric", &MetricSet::metric);
    _lua.new_usertype<MetricValue>( "MetricValue",
                                    "ndims", &MetricValue::ndims,
                                    "value", &MetricValue::value,
                                    "zscore", &MetricValue::zscore,
                                    "mean", &MetricValue::mean);
    _lua.new_usertype<GrowthData>( "GrowthData",
                                   "source", &GrowthData::source,
                                   "note", &GrowthData::note,
                                   "longNote", &GrowthData::longNote);
    _lua.new_usertype<FM>( "FM",
                           "age", &FM::age,
                           "sex", &FM::sex,
                           "studyId", &FM::studyId,
                           "maternalEthnicity", &FM::maternalEthnicity,
                           "paternalEthnicity", &FM::paternalEthnicity,
                           "captureDate", &FM::captureDate,
                           "dateOfBirth", &FM::dateOfBirth,
                           "currentAssessment", &FM::cassessment,
                           "hasLandmarks", &FM::hasLandmarks,
                           "hasMask", &FM::hasMask,
                           "maskHash", &FM::maskHash);
    _lua.new_usertype<FaceAssessment>( "FaceAssessment",
                           "hasLandmarks", &FaceAssessment::hasLandmarks,
                           "metrics", &FaceAssessment::cmetrics);
    _lua.new_usertype<Phenotype>( "Phenotype",
                                  "name", &Phenotype::name);
    _lua.new_usertype<QDate>( "QDate",
                              "day", QOverload<>::of(&QDate::day),
                              "month", QOverload<>::of(&QDate::month),
                              "year", QOverload<>::of(&QDate::year));
    _lua.new_usertype<QString>( "QString",
                                "toStdString", &QString::toStdString,
                                "isEmpty", &QString::isEmpty);
    auto boxType = _lua.new_usertype<QRectF>( "Box",
            sol::constructors<QRectF(), QRectF(qreal, qreal, qreal, qreal)>());
    boxType["x"] = sol::property( &QRectF::x, &QRectF::setX);
    boxType["y"] = sol::property( &QRectF::y, &QRectF::setY);
    boxType["w"] = sol::property( &QRectF::width, &QRectF::setWidth);
    boxType["h"] = sol::property( &QRectF::height, &QRectF::setHeight);

    // Standard content adding API
    _lua.set_function( "addScanInfo",
                       [this]( const QRectF &box, const FM *fm){ this->_addLatexScanInfo( box, fm);});
    _lua.set_function( "addNotes",
                       [this]( const QRectF &box, const FM *fm){ this->_addLatexNotes( box, fm);});
    _lua.set_function( "addPhenotypicTraits",
                       [this]( const QRectF &box, const FM *fm, int sidx, int nhids)
                       { return this->_addLatexPhenotypicTraits( box, fm, sidx, nhids);});
    _lua.set_function( "addFigure",
                       [this]( const QRectF &box, const FM *fm, const std::string& caption)
                       { this->_addLatexFigure( box, fm, caption);});
    _lua.set_function( "addSelectedColourMapFigure",
                       [this]( const QRectF &box, const std::string& caption)
                       { this->_addLatexSelectedColourMapFigure( box, caption);});
    _lua.set_function( "addChart",
                       [this]( const QRectF &box, const FM *fm, int mid, size_t d, int footnotemark)
                       { this->_addLatexChart( box, fm, mid, d, footnotemark);});
    _lua.set_function( "addFootnoteSources",
                       [this]( const QRectF &box, const FM *fm, const sol::table &mids)
                       { this->_addLatexFootnoteSources( box, fm, mids);});

    // Standard getters
    _lua.set_function( "getNumPhenotypicTraits",
                       []( const FM *fm)
                       { return getNumPhenotypicTraits(fm);});
    _lua.set_function( "getSelectedViewActiveColoursName",
                       []()
                       { return getSelectedViewActiveColoursName();});
    _lua.set_function( "metricSource",
                       []( const FM *fm, int mid)
                       { return metricCurrentSource( fm, mid);});
    _lua.set_function( "footnoteIndices",
                       []( const FM *fm, const sol::table &mids)
                       { return footnoteIndices( fm, mids);});
    _lua.set_function( "round",
                       []( double v, size_t nd)
                       { return rlib::round(v,nd);});
    _lua.set_function( "metric", MM::metric);
}   // end ctor


Report::~Report()
{
    if ( _ltxw)
    {
        delete _ltxw;
        _ltxw = nullptr;
    }   // end if
}   // end dtor


// Convert the box with values in [0,1] to actual
// page position in millimetres for the LatexWriter.
r3dio::Box Report::_pageBox( const QRectF &box) const
{
    const float pw = _pageDims.width(); 
    const float ph = _pageDims.height();
    return r3dio::Box( box.x() * pw, box.y() * ph, box.width() * pw, box.height() * ph);
}   // end _pageBox


void Report::addCustomLuaFn( const QString& fnName,
        const std::function<void( const QRectF&)>& fn)
{
    _lua.set_function( fnName.toStdString(), fn);
}   // end addCustomLuaFn


void Report::addCustomLuaFn( const QString& fnName,
        const std::function<void( const QRectF&, const FM*)>& fn)
{
    _lua.set_function( fnName.toStdString(), fn);
}   // end addCustomLuaFn


void Report::addLatex( const QRectF &box, const QString &s, bool centre)
{
    _ltxw->addRaw( _pageBox(box), s.toStdString(), centre);
}   // end addLatex


Report::Ptr Report::load( const QString& fname)
{
    bool loadedOk = false;
    Ptr report( new Report, []( Report* d){ delete d;});

    try
    {
        QFile file(fname);
        if ( file.open(QIODevice::ReadOnly | QIODevice::Text))  // Open for read only text
        {
            QTextStream in(&file);
            const QString code = in.readAll();
            report->_lua.script( code.toStdString());
            loadedOk = true;
        }   // end if
    }   // end try
    catch ( const sol::error& e)
    {
        qWarning() << "Unable to load and execute file '" << fname << "'!" << Qt::endl << "\t" << e.what();
    }   // end catch

    if ( !loadedOk)
        return nullptr;

    auto table = report->_lua["report"];
    if ( !table.valid())
    {
        qWarning() << "Missing table 'report'!";
        return nullptr;
    }   // end if

    if ( sol::optional<std::string> v = table["name"])
        report->_name = v.value().c_str();
    else
    {
        qWarning() << "Missing report name!";
        return nullptr;
    }   // end else

    // Reject the TEST report if this is the release build
    if ( report->name().toUpper() == "TEST")
    {
#ifndef NDEBUG
        std::cerr << "[INFO] FaceTools::Report::Report::load: Loading test report for debug version." << std::endl;
#else
        return nullptr;
#endif
    }   // end if

    if ( sol::optional<std::string> v = table["title"])
        report->_title = v.value().c_str();
    else
    {
        qWarning() << "Missing report title!";
        return nullptr;
    }   // end else

    report->_twoModels = table["twoModels"].get_or(false);

    report->_pageDims = s_pageDims; // Default page dims
    if ( const sol::optional<sol::table> v = table["pageDims"])
    {
        const sol::table pdims = v.value();
        report->_pageDims.setWidth( pdims["width"].get_or(s_pageDims.width()));
        report->_pageDims.setHeight( pdims["height"].get_or(s_pageDims.height()));
    }   // end if

    if ( sol::optional<sol::function> v = table["isAvailable"])
        report->_isAvailable = v.value();
    else
    {
        qWarning() << "Missing 'available' function!";
        return nullptr;
    }   // end else

    if ( sol::optional<sol::function> v = table["setContent"])
        report->_setContent = v.value();
    else
    {
        qWarning() << "Missing 'setContent' function!";
        return nullptr;
    }   // end else

    return report;
}   // end load


bool Report::isAvailable() const
{
    const FM *fm0 = MS::selectedModel();
    const FM *fm1 = MS::nonSelectedModel();
    if ( !fm0 || (_twoModels && !fm1))
        return false;

    bool available = false;
    try
    {
        if (_twoModels && fm1)
        {
            sol::function_result result = _isAvailable( fm0, fm1);
            if ( result.valid())
                available = result;
        }   // end if
        else
        {
            sol::function_result result = _isAvailable( fm0);
            if ( result.valid())
                available = result;
        }   // end else
    }   // end try
    catch (const sol::error& e)
    {
        qWarning() << "Lua Error!:" << e.what();
        available = false;
    }   // end catch
    return available;
}   // end isAvailable


bool Report::setContent()
{
    _errMsg = "";
    if ( _ltxw)
        delete _ltxw;
    _ltxw = new LatexWriter;
    if ( !_ltxw->open( _pageDims.width(), _pageDims.height()))
    {
        _errMsg = tr( "Failed to open report writing stream!");
        return false;
    }   // end if
    _validContent = _writeLatex();
    if ( !_validContent)
        _errMsg = tr( "Failed to set report contents!");
    return _validContent;
}   // end setContent


bool Report::generate()
{
    _errMsg = "";
    if ( !_ltxw)
    {
        _errMsg = tr( "Must set report content before generating!");
        return false;
    }   // end if
    std::string outpdf;
    if ( !_ltxw->makePDF( outpdf))
        _errMsg = tr("Failed to generate the report PDF!");
    _pdffile = QString::fromStdString(outpdf);
    if ( !_pdffile.isEmpty())
        std::cerr << "Report complete at \"" << _pdffile.toLocal8Bit().toStdString() << "\"" << std::endl;
    return _errMsg.isEmpty();
}   // end generate


bool Report::_writeLatex()
{
    assert(_ltxw);
    LatexWriter &ltxw = *_ltxw;
    const BFS::path workdir = ltxw.workingDirectory();
    std::cerr << "Report working dir \"" << workdir.string() << "\"" << std::endl;

    if ( _usingSVG())
    {
    //    ltxw << "\\usepackage[inkscapeexe=\"" << s_inkscape.toStdString() << "\"]{svg}\n"
    //         << "\\usepackage{relsize}\n";
        ltxw << "\\usepackage{svg}\n"   // graphicx also included by svg
           << "\\setsvg{inkscape={\"" << s_inkscape.toStdString() << "\"}}\n"
           << "\\usepackage{relsize}\n";
    }   // end if

    // Write the header. Copy in the logo to the working directory.
    // Note that this is a resource so need to use Qt's file copy.
    const std::string logopdf = "logo.pdf";
    const QString logopath = QString::fromStdString( (workdir / logopdf).string());
    if ( !QFile( s_logoPath).copy( logopath))
        return false;

    ltxw << "\\usepackage{footnote}\n"
       << "\\usepackage{fancyhdr}\n"
       << "\\pagestyle{fancy}\n"
       << "\\setlength\\headheight{35mm}\n"
       << "\\fancyhfoffset[L]{-10mm}\n"
       << "\\fancyhfoffset[R]{-10mm}\n"
       << "\\fancyhead[L]{\\Large \\textbf{" << sanit(_title) << "} \\\\ \n"
       << "\\vspace{2mm}\n" // Small gap between title and date
       << "\\normalsize \\textbf{Report Date:} " << sanit(QDate::currentDate().toString("dd MMM yyyy"))
       << "\\vspace{2mm}}\n"
       << "\\fancyhead[R]{\\raisebox{0mm}{\\includegraphics[width=45mm]{" << logopdf << "}} \\\\ \n"
       << "\\footnotesize " << sanit("Version " + s_versionStr) << "}\n";

    ltxw.beginDocument();
    _validContent = true;
    try
    {
        const FM *fm0 = MS::selectedModel();
        assert( fm0);
        const FM *fm1 = MS::nonSelectedModel();
        if (_twoModels && fm1)
            _setContent( fm0, fm1);   // Lua call to add report elements
        else
            _setContent( fm0);
    }   // end try
    catch (const sol::error& e)
    {
        qWarning() << "Lua Error: " << e.what();
        _validContent = false;
    }   // end catch
    ltxw.endDocument();
    ltxw.close();
    return _validContent;
}   // end _writeLatex


std::string Report::_writeModelBGImage( const QRectF &box, const FM *fm, const r3d::Mesh &mesh)
{
    const float pw = _pageDims.width(); 
    const float ph = _pageDims.height();
    // Background image for model until user enables 3D content to replace this.
    const float RES = 72.0f/25.4f;  // Pixels per mm
    const QSize bimSz( box.width() * pw * RES, box.height() * ph * RES);
    const cv::Mat img = Action::ActionUpdateThumbnail::generateImage( fm, mesh, bimSz, 30, 0.8f);
    BFS::path imgFile = BFS::unique_path( "%%%%-%%%%-%%%%-%%%%.jpg");
    const BFS::path workDir = _ltxw->workingDirectory();
    // Save the image into the working directory
    if ( !cv::imwrite( (workDir/imgFile).string(), img))
        imgFile = "";
    return imgFile.string();
}   // end _writeModelBGImage


std::string Report::_writeModelBGImage( const QRectF &box, const FM *fm)
{
    return _writeModelBGImage( box, fm, fm->mesh());
}   // end _writeModelBGImage


void Report::_addLatexFigure( const QRectF &box, const FM *fm, const std::string &caption)
{
    assert(fm);
    if ( !_validContent)
        return;

    // Copy the cached U3D model to the Latex working directory
    // to allow timely release of the U3D cache lock.
    const std::string u3dfile = BFS::unique_path( "%%%%-%%%%-%%%%-%%%%.u3d").string();
    if ( !_ltxw->copyInFile( U3DCache::u3dfilepath(*fm)->toStdString(), u3dfile))
    {
        _validContent = false;
        return;
    }   // end if

    const std::string imgfile = _writeModelBGImage( box, fm);
    if ( imgfile.empty())
    {
        _validContent = false;
        return;
    }   // end if

    const auto cam = Action::ActionOrientCamera::makeFrontCamera( *fm, 30, 0.8f);
    _ltxw->addMesh( _pageBox(box), u3dfile, cam, imgfile, caption);
}   // end _addLatexFigure


void Report::_addLatexSelectedColourMapFigure( const QRectF &box, const std::string &caption)
{
    if ( !_validContent)
        return;

    const Vis::FV *fv = MS::selectedView();
    assert(fv);
    const FM *fm = fv->data();

    const std::string u3dfile = BFS::unique_path( "%%%%-%%%%-%%%%-%%%%.u3d").string();
    const BFS::path workdir = _ltxw->workingDirectory();
    const std::string savepath = (workdir/u3dfile).string();
    r3d::Mesh::Ptr mesh = U3DCache::makeColourMappedU3D( fv, QString::fromStdString(savepath));
    if ( !mesh) // The textured mesh
    {
        _validContent = false;
        return;
    }   // end if

    const std::string imgfile = _writeModelBGImage( box, fm, *mesh);
    if ( imgfile.empty())
    {
        _validContent = false;
        return;
    }   // end if

    const auto cam = Action::ActionOrientCamera::makeFrontCamera( *fm, 30, 0.8f);
    _ltxw->addMesh( _pageBox(box), u3dfile, cam, imgfile, caption);
}   // end _addLatexSelectedColourMapFigure


void Report::_addLatexChart( const QRectF &box, const FM *fm, int mid, size_t d, int fnm)
{
    if ( !_validContent)
        return;

    MC::CPtr mc = MM::metric(mid);
    assert(mc);
    if ( !mc)
    {
        _validContent = false;
        return;
    }   // end if

    // Define the resolution, position, and aspect ratio of the chart.
    // All these disgusting fudge factors are necessary to have the
    // chart fit snuggly within the defined image bounds.
    const float pw = _pageDims.width(); 
    const float ph = _pageDims.height();
    const float DPI = 144.0f/25.4f;
    const QSize SZ( box.width() * pw * DPI, box.height() * ph * DPI);
    const float pamw = SZ.width()*0.10f;
    const float pamh = SZ.height()*0.11f;
    Metric::Chart *chart = new Metric::Chart( mid, d, fm);
    chart->setMargins( QMargins(0,0,0,0));  // Left, top, right, bottom - necessary
    chart->setGeometry( 0,0,SZ.width()-pamw,SZ.height());
    QtCharts::QChartView cview( chart); // ChartView takes ownership of chart
    cview.setFixedSize( SZ.width(), SZ.height());
    cview.fitInView( 0.2f*pamw,0.25f*pamh,SZ.width()-1.6f*pamw,SZ.height()-1.6f*pamh);

    // Ensure unique chart filename (may need several per report).
    const bool usingSVG = _usingSVG();
    const std::string imgname = BFS::unique_path().string() + (usingSVG ? ".svg" : ".png");
    const BFS::path workdir = _ltxw->workingDirectory();
    QString imgpath = QString::fromStdString((workdir/imgname).string());

    if ( !Widget::ChartDialog::saveImage( &cview, imgpath))
    {
        std::cerr << "[ERROR] FaceTools::Report::_addLatexChart: Unable to save image!" << std::endl;
        _validContent = false;
        return;
    }   // end if

    const r3dio::Box pgbox = _pageBox(box);
    std::ostringstream oss;
    oss << "\\begin{figure}\n";
    oss << "\\footnotesize\n";
    if ( fnm >= 1)
        oss << "\\caption*{" << chart->makeLatexTitleString( fnm).toStdString() << "}\n";
    oss << (usingSVG ? "\\includesvg" : "\\includegraphics")
        << "[width=" << pgbox[2] << "mm,height=" << pgbox[3] << "mm]{" << imgname << "}\n";
    oss << "\\normalsize\n";
    oss << "\\end{figure}\n";
    _ltxw->addRaw( pgbox, oss.str(), true);
}   // end _addLatexChart


void Report::_addLatexScanInfo( const QRectF &box, const FM *fm)
{
    if ( !_validContent)
        return;

    const std::string cdate = sanit( fm->captureDate().toString("dd MMM yyyy"));
    const std::string dob = sanit( fm->dateOfBirth().toString("dd MMM yyyy"));
    const double age = fm->age();
    const int yrs = int(age);
    const int mths = int((age - double(yrs)) * 12);
    const std::string sage = sanit(QString("%1 yrs. %2 mons.").arg(yrs).arg(mths));

    const std::string sexs = sanit(FaceTools::toSexString( fm->sex())); // F/M/F M
    std::string eths = sanit(Ethnicities::name(fm->maternalEthnicity()));
    if ( fm->maternalEthnicity() != fm->paternalEthnicity())
        eths += sanit(" (M) & " + Ethnicities::name(fm->paternalEthnicity()) + " (P)");
    if ( eths.empty())
        eths = "N/A";

    std::ostringstream oss;
    // Source and Study reference
    const std::string src = sanit(fm->source().isEmpty() ? "N/A" : fm->source());
    const std::string studyId = sanit(fm->studyId().isEmpty() ? "N/A" : fm->studyId());
    oss << "\\textbf{Source:} " << src << "\\hspace{2mm}\\textbf{Study Ref:} " << studyId << " \\\\" << std::endl;
    // Subject and Image reference
    const std::string subRef = sanit(fm->subjectId().isEmpty() ? "N/A" : fm->subjectId());
    const std::string imgRef = sanit(fm->imageId().isEmpty() ? "N/A" : fm->imageId());
    oss << "\\textbf{Subject Ref:} " << subRef << "\\hspace{2mm}\\textbf{Image Ref:} " << imgRef << " \\\\" << std::endl;
    oss << "\\textbf{Image Date:} " << cdate << " \\\\" << std::endl;
    oss << "\\textbf{Birth Date:} " << dob << "\\hspace{2mm} \\textbf{Age:} " << sage << " \\\\" << std::endl;
    oss << "\\textbf{Sex:} " << sexs << "\\hspace{2mm} \\textbf{Ethnicity:} " << eths << std::endl;
    _ltxw->addRaw( _pageBox(box), oss.str());
}   // end _addLatexScanInfo


void Report::_addLatexNotes( const QRectF &box, const FM *fm)
{
    if ( !_validContent)
        return;

    FaceAssessment::CPtr ass = fm->currentAssessment();
    std::ostringstream oss;
    oss << "\\normalsize{\\textbf{" << sanit(tr("Assessment")) << ":} " << sanit(ass->assessor()) << "} \\\\" << std::endl;
    oss << "\\small{" << sanit( ass->hasNotes() ? ass->notes() : tr("Nothing recorded.")) << "} \\\\" << std::endl;
    _ltxw->addRaw( _pageBox(box), oss.str());
}   // end _addLatexNotes


bool Report::_addLatexPhenotypicTraits( const QRectF &box, const FM *fm, int sidx, int nhids)
{
    if ( !_validContent)
        return false;

    sidx = std::max( 0, sidx);
    const IntSet pids = PhenotypeManager::discover(*fm, -1); // Use the current assessment id
    bool moreToGo = false;

    std::ostringstream oss;
    if ( pids.empty())
        oss << sanit(tr("No notable morphology found.")) << std::endl;
    else
    {
        if ( int(pids.size()) <= sidx)
            return false;

        const int topidx = std::min<int>( int(pids.size()), sidx + nhids);
        if ( int(pids.size()) > topidx)
            moreToGo = true;

        std::list<int> lids( pids.begin(), pids.end());
        lids.sort();
        std::vector<int> vids( lids.begin(), lids.end());
        oss << "\\textbf{" << sanit(tr("Noted phenotypic traits")) << ":}" << std::endl
            << "\\begin{itemize}" << std::endl;
        for ( int i = sidx; i < topidx; ++i)
        {
            const std::string itemstr = PhenotypeManager::latexLinkString(vids[i]).toStdString();
            oss << "\\item " << itemstr << std::endl;
        }   // end for
        oss << "\\end{itemize}" << std::endl;
    }   // end else

    _ltxw->addRaw( _pageBox(box), oss.str());
    return moreToGo;
}   // end _addLatexPhenotypicTraits


void Report::_addLatexFootnoteSources( const QRectF &box, const FM *fm, const sol::table& mids)
{
    if ( !_validContent)
        return;

    std::ostringstream oss;
    std::unordered_set<std::string> refs;
    int idx = 1;
    for ( size_t i = 0; i < mids.size(); ++i)
    {
        const int mid = mids[i];
        const std::string src = metricCurrentSource( fm, mid);
        if ( !src.empty() && refs.count(src) == 0)
        {
            refs.insert(src);
            oss << "\\footnotetext[" << idx << "]{" << src << "}" << std::endl;
            idx++;
        }   // end if
    }   // end for
    _ltxw->addRaw( _pageBox(box), oss.str());
}   // end _addLatexFootnoteSources
