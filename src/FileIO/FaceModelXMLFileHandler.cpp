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

#include <FaceModelXMLFileHandler.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <PhenotypeManager.h>
#include <Ethnicities.h>
#include <MiscFunctions.h>
#include <AssetImporter.h>
#include <OBJExporter.h>
#include <Orientation.h>    // RFeatures
#include <FileIO.h>         // rlib
#include <QTemporaryDir>
#include <quazip5/JlCompress.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <ctime>
using FaceTools::FileIO::FaceModelXMLFileHandler;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using FaceTools::FM;

void FaceModelXMLFileHandler::exportMetaData( const FM* fm, const std::string& objfname, PTree& tnode)
{
    PTree& rnode = tnode.add("FaceModel","");

    rnode.put( "ObjFilename", objfname);
    rnode.put( "Notes", fm->notes().toStdString());
    rnode.put( "Source", fm->source().toStdString());
    rnode.put( "StudyId", fm->studyId().toStdString());
    rnode.put( "DateOfBirth", fm->dateOfBirth().toString().toStdString());
    rnode.put( "Sex", FaceTools::toSexString(fm->sex()).toStdString());
    rnode.put( "MaternalEthnicity", fm->maternalEthnicity());
    rnode.put( "PaternalEthnicity", fm->paternalEthnicity());
    rnode.put( "CaptureDate", fm->captureDate().toString().toStdString());

    fm->landmarks().write( rnode.put("Landmarks", ""));
    fm->paths().write( rnode.put("Paths", ""));

    rnode.put("Orientation", "") << fm->orientation();
    RFeatures::putVertex( rnode.put("Centre", ""), fm->icentre());

    const double age = fm->age();
    PTree& mgrps = rnode.put("MetricGroups", "");
    fm->cmetrics().write( mgrps.put("Frontal", ""), age);
    fm->cmetricsL().write( mgrps.put("LeftLateral", ""), age);
    fm->cmetricsR().write( mgrps.put("RightLateral", ""), age);

    PTree& hpos = rnode.put("PhenotypicIndications", "");
    const IntSet ptypes = PhenotypeManager::discover(fm);
    for ( int hid : ptypes)
    {
        PTree& node = hpos.add( "Term", "");
        const Phenotype::Ptr pind = PhenotypeManager::phenotype(hid);

        node.put( "Id", pind->id());
        node.put( "Name", pind->name().toStdString());

        QStringList mids;
        for ( int mid : pind->metrics())
            mids << QString("%1").arg(mid);
        const QString midList = mids.join(",");
        node.put( "Metrics", midList.toStdString());

        node.put( "SexMatch", pind->isSexMatch(fm->sex()));
        node.put( "AgeMatch", pind->isAgeMatch(fm->age()));
        node.put( "MaternalEthnicityMatch", pind->isEthnicityMatch(fm->maternalEthnicity()));
        node.put( "PaternalEthnicityMatch", pind->isEthnicityMatch(fm->paternalEthnicity()));
    }   // end for
}   // end exportMetaData


// private static
PTree& FaceModelXMLFileHandler::exportXMLHeader( PTree& tree)
{
    PTree& topNode = tree.put( "faces","");
    topNode.put( "<xmlattr>.version", XML_VERSION);
    std::ostringstream desc;
    desc << XML_FILE_DESCRIPTION << ";" << time(nullptr);
    topNode.put( "description", desc.str());

    PTree& records = topNode.put( "FaceModels","");    // Only a single record
    records.put( "<xmlattr>.count", 1);
    return records;
}   // end exportXMLHeader


namespace {

void importModelRecord( FM* fm, const PTree& rnode, std::string& objfilename)
{
    if ( rnode.count("objfilename") > 0)
        objfilename = rnode.get<std::string>( "objfilename");  // Without path info
    else if ( rnode.count("ObjFilename") > 0)
        objfilename = rnode.get<std::string>( "ObjFilename");  // Without path info

    QString notes;
    if ( rnode.count("description") > 0)
        notes = rnode.get<std::string>( "description").c_str();
    else if ( rnode.count("Description") > 0)
        notes = rnode.get<std::string>( "Description").c_str();
    else if ( rnode.count("Notes") > 0)
        notes = rnode.get<std::string>( "Notes").c_str();
    fm->setNotes(notes);

    QString src;
    if ( rnode.count("source") > 0)
        src = rnode.get<std::string>( "source").c_str();
    else if ( rnode.count("Source") > 0)
        src = rnode.get<std::string>( "Source").c_str();
    fm->setSource(src);

    QString sid;
    if ( rnode.count("studyid") > 0)
        sid = rnode.get<std::string>( "studyid").c_str();
    else if ( rnode.count("StudyId") > 0)
        sid = rnode.get<std::string>( "StudyId").c_str();
    fm->setStudyId(sid);

    QDate cdate = QDate::currentDate();
    if ( rnode.count("capture_date") > 0)
        cdate = QDate::fromString( rnode.get<std::string>( "capture_date").c_str());
    else if ( rnode.count("CaptureDate") > 0)
        cdate = QDate::fromString( rnode.get<std::string>( "CaptureDate").c_str());
    fm->setCaptureDate( cdate);

    double age = 0.0;   // Only use age if DateOfBirth not present in XML
    if ( rnode.count("age") > 0)
        age = rnode.get<double>("age");
    else if ( rnode.count("Age") > 0)
        age = rnode.get<double>("Age");

    QDate dob = QDate::currentDate();
    if ( rnode.count("DateOfBirth") > 0)
        dob = QDate::fromString( rnode.get<std::string>( "DateOfBirth").c_str());

    // If no date of birth given, find it as the capture date minus the age (assumed given)
    if ( dob == QDate::currentDate() && cdate != QDate::currentDate())
        dob = cdate.addDays(qint64( -age * 365.25));
    fm->setDateOfBirth( dob);

    int8_t sex = FaceTools::UNKNOWN_SEX;
    if ( rnode.count("sex") > 0)
        sex = FaceTools::fromSexString( rnode.get<std::string>("sex").c_str());
    else if ( rnode.count("Sex") > 0)
        sex = FaceTools::fromSexString( rnode.get<std::string>("Sex").c_str());
    fm->setSex(sex);

    int methn = 0;
    int pethn = 0;
    if ( rnode.count("ethnicity") > 0)
    {
        QString seth = rnode.get<std::string>( "ethnicity").c_str();
        seth = seth.split(" ")[0].trimmed();  // Use just the first word
        pethn = methn = FaceTools::Ethnicities::code( seth);
    }   // end if
    else if ( rnode.count("Ethnicity") > 0)
    {
        QString seth = rnode.get<std::string>( "Ethnicity").c_str();
        seth = seth.split(" ")[0].trimmed();  // Use just the first word
        pethn = methn = FaceTools::Ethnicities::code( seth);
    }   // end else if

    if ( rnode.count("MaternalEthnicity") > 0)
        methn = rnode.get<int>( "MaternalEthnicity");

    if ( rnode.count("PaternalEthnicity") > 0)
        pethn = rnode.get<int>( "PaternalEthnicity");

    fm->setMaternalEthnicity(methn);
    fm->setPaternalEthnicity(pethn);

    // Read in the landmarks
    FaceTools::Landmark::LandmarkSet::Ptr lmks = FaceTools::Landmark::LandmarkSet::create();
    if ( rnode.count("Landmarks") > 0)
    {
        lmks->read( rnode.get_child("Landmarks"));
        fm->setLandmarks(lmks);
    }   // end if

    // Read in the saved paths
    FaceTools::PathSet::Ptr paths = FaceTools::PathSet::create();
    if ( rnode.count("Paths") > 0)
    {
        paths->read( rnode.get_child("Paths"));
        fm->setPaths(paths);
    }   // end if
}   // end importModelRecord
}   // end namespace


double FaceModelXMLFileHandler::importMetaData( FM* fm, const PTree& tree, std::string& objfilename)
{
    static const std::string msghd( " FaceTools::FileIO::FaceModelXMLFileHandler::importMetaData: ");

    std::string vstr;
    const PTree* fnode = nullptr; // Will point to the FaceModel record

    if ( tree.count("faces") > 0)
    {
        const PTree& faces = tree.get_child( "faces");
        if ( faces.count("<xmlattr>.version") > 0)
            vstr = faces.get<std::string>( "<xmlattr>.version");

        std::string filedesc;
        if ( faces.count("description") > 0)
            filedesc = faces.get<std::string>( "description");
        else if ( faces.count("Description") > 0)
            filedesc = faces.get<std::string>( "Description");

        if ( !filedesc.empty())
            std::cerr << "[INFO]" << msghd << "\"" << filedesc << "\"" << std::endl;

        if ( faces.count("FaceModels") > 0)
        {
            const PTree& fms = faces.get_child( "FaceModels");
            if ( fms.count("FaceModel") > 0)
                fnode = &fms.get_child("FaceModel");
        }   // end if
        else if ( faces.count("FaceModel") > 0)
            fnode = &faces.get_child("FaceModel");
    }   // end if
    else if ( tree.count("FaceModel") > 0)
        fnode = &tree.get_child("FaceModel");

    if ( fnode)
    {
        if ( fnode->count("<xmlattr>.version") > 0)
            vstr = fnode->get<std::string>( "<xmlattr>.version");
    }   // end if
    else
    {
        std::cerr << "[WARNING]" << msghd << "FaceModel record not found!" << std::endl;
        return -1;
    }   // end else

    static const double VERSION = QString( XML_VERSION.c_str()).toDouble();

    double fversion = VERSION;
    if ( !vstr.empty())
        fversion = QString(vstr.c_str()).toDouble();

    // If the file version is lower than this library version then print a warning (should be backwards compatible).
    // But if the file version is higher, then compatibility may not be possible (still try to read in).
    if ( fversion < VERSION)
        std::cerr << "[INFO]" << msghd << "Lower version " << fversion << " of XML file being read into version " << VERSION << " library." << std::endl;
    else if ( fversion > VERSION)
        std::cerr << "[WARNING]" << msghd << "Higher version " << fversion << " of XML file being read into version " << VERSION << " library!" << std::endl;

    importModelRecord( fm, *fnode, objfilename);

    return fversion;
}   // end importMetaData


// public
FM* FaceModelXMLFileHandler::read( const QString& sfname)
{
    _err = "";
    _fversion = 0.0;
    FM* fm = nullptr;

    try
    {
        QTemporaryDir tdir;
        if ( !tdir.isValid())
        {
            _err = "Unable to open temporary directory for reading from!";
            return nullptr;
        }   // end if

        QStringList fnames = JlCompress::extractDir( sfname, tdir.path());
        if ( fnames.isEmpty())
        {
            _err = "Unable to extract files from archive!";
            return nullptr;
        }   // end if

        QStringList xmlList = QDir( tdir.path()).entryList( {"*.xml"});
        std::string xmlfile;
        if ( xmlList.size() == 1)
            xmlfile = tdir.filePath( xmlList.first()).toStdString();

        if ( xmlfile.empty() || !boost::filesystem::is_regular_file( xmlfile))
        {
            _err = "Cannot find XML meta-data in archive!";
            return nullptr;
        }   // end if

        std::ifstream ifs;
        ifs.open( xmlfile);
        if ( !ifs.is_open())
        {
            std::ostringstream serr;
            serr << "Unable to open \"" << xmlfile << "\" for reading!";
            _err = serr.str().c_str();
            return nullptr;
        }   // end if

        PTree tree;
        boost::property_tree::read_xml( ifs, tree);
        fm = new FM;
        std::string objfilename;
        const double fvers = importMetaData( fm, tree, objfilename);

        if ( fvers > 0)
        {
            _fversion = fvers;
            const std::string fext = rlib::getExtension( objfilename);
            assert( fext == "obj");
            RModelIO::AssetImporter importer(true,true);
            importer.enableFormat(fext);
            const std::string objfile = tdir.filePath( objfilename.c_str()).toStdString();
            std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::read: Loading model from \"" << objfile << "\"" << std::endl;
            RFeatures::ObjModel::Ptr model = importer.load( objfile);   // Raw model - no post process undertaken!
            if ( model)
                fm->update( model);
            else
            {
                _err = QString("Couldn't load model data from '%1'").arg(objfile.c_str());
                delete fm;
                fm = nullptr;
            }   // end if
        }   // end if
        else
        {
            if ( fvers == 0.0)
                _err = "No FaceModel objects recorded in file!";
            else
                _err = "Invalid record header in file!";
            delete fm;
            fm = nullptr;
        }   // end else
    }   // end try
    catch ( const boost::property_tree::ptree_bad_path& e)
    {
        _err = "XML bad path error encountered reading in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const boost::property_tree::xml_parser_error& e)
    {
        _err = "XML parse error encountered reading in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const std::exception& e)
    {
        _err = "Unable to read in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch

    if ( !_err.isEmpty() && fm)
    {
        delete fm;
        fm = nullptr;
    }   // end if

    return fm;
}   // end read



// public
bool FaceModelXMLFileHandler::write( const FM* fm, const QString& sfname)
{
    assert(fm);
    _err = "";

    try
    {
        std::string fname = sfname.toStdString();   // /home/rich/Desktop/13040011.3df
        QTemporaryDir tdir( boost::filesystem::path(fname).parent_path().string().c_str());
        if ( !tdir.isValid())
        {
            _err = "Unable to create temporary directory for writing to!";
            return false;
        }   // end if

        std::string xmlfile = boost::filesystem::path(fname).filename().replace_extension( "xml").string();   // 13040011.xml
        xmlfile = tdir.filePath( xmlfile.c_str()).toStdString();

        std::ofstream ofs;
        ofs.open( xmlfile);
        if ( !ofs.is_open())
        {
            _err = ("Unable to open \"" + xmlfile + "\" for writing!").c_str();
            return false;
        }   // end if

        // Write out the model geometry itself into .obj format.
        RModelIO::OBJExporter exporter;
        const std::string modfile = boost::filesystem::path(xmlfile).replace_extension( "obj").string();
        const RFeatures::ObjModel& model = fm->model();
        if ( !exporter.save( model, modfile))
        {
            _err = exporter.err().c_str();
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed! " << _err.toStdString() << std::endl;
            return false;
        }   // end if

        std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported object to " << modfile << std::endl;

        // Export jpeg thumbnail of model.
        const std::string thumbfile = boost::filesystem::path(xmlfile).replace_extension( "jpg").string();
        cv::Mat img = FaceTools::makeThumbnail( fm, cv::Size(256,256), 500);   // Generate thumbnail
        cv::imwrite( thumbfile, img);

        PTree tree;
        PTree& rnode = exportXMLHeader( tree);
        const std::string relfname = boost::filesystem::path( modfile).filename().string(); // Relative filepath
        exportMetaData( fm, relfname, rnode);
        boost::property_tree::write_xml( ofs, tree);
        ofs.close();

        std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported meta-data to " << xmlfile << std::endl;

        // Finally, zip up the contents of the directory into sfname.
        if ( !JlCompress::compressDir( sfname, tdir.path(), true/*recursively pack subdirs*/))
            _err = "Unable to compress saved data into archive format!";
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed to write to "
            << sfname.toStdString() << std::endl;
        _err = e.what();
    }   // end catch

    return _err.isEmpty();
}   // end write
