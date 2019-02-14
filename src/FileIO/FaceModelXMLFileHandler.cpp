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

#include <FaceModelXMLFileHandler.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <PhenotypeManager.h>
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

namespace {

void writeFaceModel( const FM* fm, const std::string& objfname, PTree& rnode)
{
    const std::string relfname = boost::filesystem::path( objfname).filename().string(); // Relative filepath
    rnode.put( "ObjFilename", relfname);
    rnode.put( "Notes", fm->notes().toStdString());
    rnode.put( "Source", fm->source().toStdString());
    rnode.put( "StudyId", fm->studyId().toStdString());
    rnode.put( "Age", fm->age());
    rnode.put( "Sex", FaceTools::toSexString(fm->sex()).toStdString());
    rnode.put( "Ethnicity", fm->ethnicity().toStdString());
    rnode.put( "CaptureDate", fm->captureDate().toString().toStdString());

    PTree& onode = rnode.put("Orientation", "");
    onode << fm->orientation();

    if ( fm->centreSet())
        RFeatures::putNamedVertex( rnode, "Centre", fm->centre());

    fm->landmarks()->write( rnode.put("Landmarks", ""));
    fm->paths()->write( rnode.put("Paths", ""));

    PTree& mgrps = rnode.put("MetricGroups", "");
    mgrps.add("Frontal", "") << fm->cmetrics();
    mgrps.add("LeftLateral", "") << fm->cmetricsL();
    mgrps.add("RightLateral", "") << fm->cmetricsR();

    PTree& hpos = rnode.put("CriteriaMatchedPhenotypes", "");
    for ( int hid : fm->phenotypes())
    {
        PTree& node = hpos.add( "HPOTerm", "");
        node.put( "<xmlattr>.id", hid);
        const Phenotype::Ptr hpo = PhenotypeManager::phenotype(hid);
        node.put( "<xmlattr>.name", hpo->name().toStdString());
        node.put( "ValidDemographics", hpo->isDemographicMatch(fm));
    }   // end for
}   // end writeFaceModel


FM* readFaceModel( const PTree& rnode, std::string& objfilename)
{
    FM* fm = new FM;

    if ( rnode.count("objfilename") > 0)
        objfilename = rnode.get<std::string>( "objfilename");  // Without path info
    else if ( rnode.count("ObjFilename") > 0)
        objfilename = rnode.get<std::string>( "ObjFilename");  // Without path info

    if ( rnode.count("description") > 0)
        fm->setNotes( rnode.get<std::string>( "description").c_str());
    else if ( rnode.count("Description") > 0)
        fm->setNotes( rnode.get<std::string>( "Description").c_str());
    else if ( rnode.count("Notes") > 0)
        fm->setNotes( rnode.get<std::string>( "Notes").c_str());

    if ( rnode.count("source") > 0)
        fm->setSource( rnode.get<std::string>( "source").c_str());
    else if ( rnode.count("Source") > 0)
        fm->setSource( rnode.get<std::string>( "Source").c_str());

    if ( rnode.count("studyid") > 0)
        fm->setStudyId( rnode.get<std::string>( "studyid").c_str());
    else if ( rnode.count("StudyId") > 0)
        fm->setStudyId( rnode.get<std::string>( "StudyId").c_str());

    if ( rnode.count("age") > 0)
        fm->setAge( rnode.get<double>("age"));
    else if ( rnode.count("Age") > 0)
        fm->setAge( rnode.get<double>("Age"));

    if ( rnode.count("sex") > 0)
        fm->setSex( FaceTools::fromSexString( rnode.get<std::string>("sex").c_str()));
    else if ( rnode.count("Sex") > 0)
        fm->setSex( FaceTools::fromSexString( rnode.get<std::string>("Sex").c_str()));

    if ( rnode.count("ethnicity") > 0)
        fm->setEthnicity( rnode.get<std::string>( "ethnicity").c_str());
    else if ( rnode.count("Ethnicity") > 0)
        fm->setEthnicity( rnode.get<std::string>( "Ethnicity").c_str());

    if ( rnode.count("capture_date") > 0)
        fm->setCaptureDate( QDate::fromString( rnode.get<std::string>( "capture_date").c_str()));
    else if ( rnode.count("CaptureDate") > 0)
        fm->setCaptureDate( QDate::fromString( rnode.get<std::string>( "CaptureDate").c_str()));

    if ( rnode.count("Orientation") > 0)
    {
        RFeatures::Orientation on;
        rnode.get_child("Orientation") >> on;
        fm->setOrientation(on);
    }   // end if

    cv::Vec3f c(0,0,0); // Get the centre if in the file
    bool gotcentre = RFeatures::getNamedVertex( rnode, "Centre", c);

    // Read in the landmarks
    FaceTools::Landmark::LandmarkSet::Ptr lmks = fm->landmarks();
    if ( rnode.count("Landmarks") > 0)
        lmks->read( rnode.get_child("Landmarks"));

    // Calculate the centre from the landmarks if unable to retrieve from the file
    if (!gotcentre && FaceTools::hasCentreLandmarks( *lmks))
    {
        c = FaceTools::calcFaceCentre( *lmks);
        gotcentre = true;
    }   // end if

    // Set the centre if read or calculated, otherwise it'll just be the middle of the largest component.
    if ( gotcentre)
        fm->setCentre(c);

    // Read in the saved paths
    if ( rnode.count("Paths") > 0)
        fm->paths()->read( rnode.get_child("Paths"));

    fm->setSaved(true);
    return fm;
}   // end readFaceModel

}   // end namespace


// public
FM* FaceModelXMLFileHandler::read( const QString& sfname)
{
    _err = "";
    _fversion = 0.0;
    FM* fm = nullptr;

    try
    {
        QTemporaryDir dir;
        if ( !dir.isValid())
        {
            _err = "Unable to open temporary directory for reading from!";
            return nullptr;
        }   // end if

        QStringList fnames = JlCompress::extractDir( sfname, dir.path());
        if ( fnames.isEmpty())
        {
            _err = "Unable to extract files from archive!";
            return nullptr;
        }   // end if

        std::string xmlfile = boost::filesystem::path(sfname.toStdString()).filename().replace_extension( "xml").string();   // 13040011.xml
        xmlfile = dir.filePath( xmlfile.c_str()).toStdString();
        if ( !boost::filesystem::exists( xmlfile) || !boost::filesystem::is_regular_file( xmlfile))
        {
            _err = "Cannot find XML meta-data file in archive!";
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

        const PTree& faces = tree.get_child( "faces");
        _fversion = QString(faces.get<std::string>( "<xmlattr>.version").c_str()).toDouble();
        if ( _fversion < QString( XML_VERSION.c_str()).toDouble())
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::read: Lower version of XML file being read!" << std::endl;

        const std::string filedesc = faces.get<std::string>( "description");
        std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::read: Reading file with description \""
                  << filedesc << "\"" << std::endl;

        const PTree& records = faces.get_child( "FaceModels");
        const int nrecs = records.get<int>( "<xmlattr>.count");
        if ( nrecs <= 0)
        {
            _err = "No FaceModel objects recorded in file!";
            return nullptr;
        }   // end if

        if ( nrecs > 1)
        {
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::read: Multiple FaceModel records not supported!"
                      << " Ignoring all records after the first." << std::endl;
        }   // end if

        std::string objfilename;
        for ( const PTree::value_type& rnode : records)
        {
            if ( rnode.first == "FaceModel")
            {
                fm = readFaceModel( rnode.second, objfilename);
                break;
            }   // end if
        }   // end for

        if ( !fm)
        {
            _err = "Unable to read in FaceModel from file record!";
            return nullptr;
        }   // end if

        const std::string fext = rlib::getExtension( objfilename);
        if ( fext != "obj")
        {
            _err = "FaceModel has its model saved in an unsupported format!";
            delete fm;
            return nullptr;
        }   // end if

        RModelIO::AssetImporter importer(true,true);
        importer.enableFormat(fext);
        const std::string objfile = dir.filePath( objfilename.c_str()).toStdString();
        std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::read: Loading model from \"" << objfile << "\"" << std::endl;
        RFeatures::ObjModel::Ptr model = importer.load( objfile);   // Doesn't merge materials or clean!
        if ( !model)
        {
            std::ostringstream serr;
            serr << "Failed to load object from \"" << objfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return nullptr;
        }   // end if

        RFeatures::ObjModelInfo::Ptr minfo = RFeatures::ObjModelInfo::create(model);
        if ( !minfo)
        {
            std::ostringstream serr;
            serr << "Failed to clean object loaded from \"" << objfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return nullptr;
        }   // end if

        fm->update( minfo);
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
        QTemporaryDir dir( boost::filesystem::path(fname).parent_path().string().c_str());
        if ( !dir.isValid())
        {
            _err = "Unable to create temporary directory for writing to!";
            return false;
        }   // end if

        std::string xmlfile = boost::filesystem::path(fname).filename().replace_extension( "xml").string();   // 13040011.xml
        xmlfile = dir.filePath( xmlfile.c_str()).toStdString();

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
        const RFeatures::ObjModel* model = fm->info()->cmodel();
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
        PTree& topNode = tree.put( "faces","");
        topNode.put( "<xmlattr>.version", XML_VERSION);
        std::ostringstream desc;
        desc << XML_FILE_DESCRIPTION << ";" << time(nullptr);
        topNode.put( "description", desc.str());

        PTree& records = topNode.put( "FaceModels","");    // Only a single record
        records.put( "<xmlattr>.count", 1);
        PTree& rnode = records.add("FaceModel","");
        writeFaceModel( fm, modfile, rnode);

        boost::property_tree::write_xml( ofs, tree);
        ofs.close();

        std::cerr << "[INFO] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported meta-data to " << xmlfile << std::endl;

        // Finally, zip up the contents of the directory into sfname.
        if ( !JlCompress::compressDir( sfname, dir.path(), true/*recursively pack subdirs*/))
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
