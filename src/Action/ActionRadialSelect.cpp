#include <ActionRadialSelector.h>
#include <FaceTools.h>
#include <Landmarks.h>
#include <ObjMetaData.h>
#include <ObjModelVertexAdder.h>    // RFeatures
#include <ObjModelSmoother.h>       // RFeatures
#include <ObjModelHoleFiller.h>     // RFeatures
#include <cassert>
using FaceTools::Action::ActionRadialSelector;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionRadialSelector::ActionRadialSelector( const char* ifile)
    : _icon( ifile)
{
    init();
    setCheckable( true);
    setEnabled( false);
    setChecked( false);
}   // end ctor


// public virtual
void ActionRadialSelector::setControlled( FaceTools::FaceControl *fcont, bool enable)
{
    if ( enable)
        _fcont = fcont;
    else if ( _fcont == fcont)
        _fcont = NULL;

    checkChanged();
}   // end setControlled


// protected virtual
void ActionRadialSelector::doBeforeAction()
{
    assert( _fconts.size() == 1);
    FaceControl* fcont = *_fconts.begin();
    assert( fcont->isFaceDetected());

    // Create the cropper object
    const FaceTools::ObjMetaData::Ptr omd = fcont->getModel()->getObjectMeta();
    const cv::Vec3f& v = omd->landmarks()->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    const int vidx = omd->getKDTree()->find(v);
    _cropper = RFeatures::ObjModelCropper::create( omd->getModel(), v, vidx);
    setCropRadius( 2.0);    // Set the initial crop radius

    // Create the boundary view
    _bview = new FaceTools::BoundaryView( fcont);
    _bview->setBoundary( _cropper->getBoundary());

    // Link mouse input to boundary adjustment

    // Show the boundary to allow user to adjust
    _bview->setVisible( true);
    _dialog.show();
}   // end doBeforeAction


// private slot
void ActionRadialSelector::doOnRejected()
{

}   // end doOnRejected


// public virtual
bool ActionRadialSelector::doAction()
{
    assert( _fconts.size() == 1);
    FaceControl* fcont = *_fconts.begin();
    FaceModel* fmodel = fcont->getModel();
    IntSet cfids;
    if ( fmodel->getCroppingRegion( cfids) == 0)
        return false;

    std::cerr << "[INFO] FaceApp::ActionSelectRegion::operator(): Cropping..." << std::endl;
    RFeatures::ObjModel::Ptr cobj = FaceTools::crop( fmodel->getObjectMeta()->getObject(), cfids);

    std::cerr << "[INFO] FaceApp::ActionSelectRegion::operator(): Filling holes... ";
    const int nfilled = RFeatures::ObjModelHoleFiller::fillHoles( cobj) - 1;
    if ( nfilled > 0)
    {
        std::cerr << nfilled << " filled";
        FaceTools::clean( cobj);
    }   // end if
    else
        std::cerr << "none found";
    std::cerr << std::endl;

    // Increase vertex density
    const double mel = fcont->getOptions().maxEdgeLength;
    std::cerr << "[INFO] FaceApp::ActionSelectRegion::operator(): Subdividing polys so no edge longer than " << mel << "..." << std::endl;
    RFeatures::ObjModelVertexAdder vadder(cobj);
    vadder.subdivideEdges( mel);

    // Smooth
    const double sfactor = fcont->getOptions().smoothFactor;
    std::cerr << "[INFO] FaceApp::ActionSelectRegion::operator(): Smoothing (" << sfactor << " factor over max 10 iterations)... ";
    RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( cobj, *cobj->getFaceIds().begin());
    size_t numSmoothIterations = 10;
    RFeatures::ObjModelSmoother( cmap).smooth( sfactor, numSmoothIterations);
    std::cerr << " done" << std::endl;

    _cobj = cobj;
    return true;
}   // end doAction


// protected virtual
void ActionRadialSelector::doAfterAction( bool rval)
{
    _dialog.hide();
    assert( _fconts.size() == 1);
    FaceControl* fcont = *_fconts.begin();
    fcont->showBoundary(false);

    if ( rval)
        fcont->updateMesh(_cobj);
    _cobj.reset();
}   // end doAfterAction


// private
void ActionRadialSelector::checkChanged()
{
    FaceControl* fcont = NULL;
    if ( !_fconts.empty())
    {
        assert( _fconts.size() == 1);
        fcont = *_fconts.begin();
    }   // end if
    setEnabled( fcont && fcont->isFaceDetected());
}   // end checkChanged


// private
void ActionRadialSelector::setCropRadius( double cRadFactor)
{
    assert( _fconts.size() == 1);
    assert( _cropper);
    FaceControl* fcont = *_fconts.begin();
    const double rad = FaceTools::calcFaceCropRadius( fcont->getModel()->getObjectMeta(), cRadFactor);
    if ( rad > 0)
        _cropper->adjustRadius( rad);
}   // end setCropRadius
