#include <FacesCohort.h>
using FaceTools::FacesCohort;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>


FacesCohort::FacesCohort( const std::string version)
{
    setVersion(version);
}   // end ctor


// public
FacesCohort::~FacesCohort()
{
    _records.clear();
}   // end dtor


// public
void FacesCohort::addRecord( ObjMetaData::Ptr fd)
{
    _records.push_back(fd);
}   // end addRecord


// public
ObjMetaData::Ptr FacesCohort::getRecord( int i) const
{
    if ( i < 0 || i >= (int)_records.size())
        return ObjMetaData::Ptr();
    return _records.at(i);
}   // end getRecord


namespace PT = boost::property_tree;

std::istream& operator>>( std::istream& is, FacesCohort& fc)
{
    try
    {
        PT::ptree tree;
        PT::read_xml( is, tree);

        const PT::ptree& faces = tree.get_child( "faces");
        if ( faces.get<std::string>( "<xmlattr>.version") != fc.version())
            throw FacesCohort::VersionException( fc.version());

        fc.setDescription( faces.get<std::string>( "description"));
        const PT::ptree& records = faces.get_child( "records");
        //const int nrecs = records.get<int>( "<xmlattr>.count");
        BOOST_FOREACH ( const PT::ptree::value_type& record, records)
        {
            if ( record.first == "record")
            {
                ObjMetaData::Ptr fd = ObjMetaData::create();
                ObjMetaData::readFrom( record.second, *fd);
                fc.addRecord( fd);
            }   // end if
        }   // end foreach
    }   // end try
    catch ( const PT::ptree_bad_path& e)
    {
        std::cerr << "XML bad path error encountered reading in stream data." << std::endl;
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const PT::xml_parser_error& e)
    {
        std::cerr << "XML parse error encountered reading in stream data." << std::endl;
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const std::exception& e)
    {
        std::cerr << "Unable to read in stream data!" << std::endl;
        std::cerr << e.what() << std::endl;
    }   // end catch

    return is;
}   // end operator>>


std::ostream& operator<<( std::ostream& os, const FacesCohort& fc)
{
    PT::ptree tree;
    PT::ptree& topNode = tree.put( "faces","");
    topNode.put( "<xmlattr>.version", fc.version());
    topNode.put( "description", fc.description());

    const std::vector<ObjMetaData::Ptr>& orecs = fc.getRecords();
    PT::ptree& records = topNode.put( "records","");
    records.put( "<xmlattr>.count", orecs.size());
    BOOST_FOREACH ( const ObjMetaData::Ptr omd, orecs)
        omd->writeTo( records);

    PT::write_xml( os, tree);
    return os;
}   // end operator<<

