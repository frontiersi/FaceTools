void FaceView::forceRebuild( const FaceModel* fmodel)
{
    setInViewer(false);
    const RFeatures::ObjModel::Ptr model = fmodel->getObjectMeta()->getObject();

    _oview = new FaceTools::OutlinesView( fmodel->getObjectMeta()->getObject());
      _bview( viewer, fmodel->getObjectMeta()),
      _lview( viewer, fmodel->getObjectMeta())

    _lview.erase();    // Erase existing!
    _lview.reset();    // Reset from ObjMetaData
    _bview.reset();    // Reset from ObjMetaData
    _oview = FaceTools::OutlinesView( _viewer, model);

    // Create the actors from the current data
    RVTK::VtkActorCreator ac;
    ac.setObjToVTKUniqueFaceMap( &_lookup);
    _sactor = ac.generateSurfaceActor( model);
    _legendRange = FaceTools::LegendRange::create( _sactor, _viewer);    // Legend mapper

    // Create the textured actor
    if ( model->getNumMaterials() > 0)
    {
        std::vector<vtkSmartPointer<vtkActor> > tactors;
        RVTK::VtkActorCreator ac2;
        ac2.generateTexturedActors( model, tactors);
        assert(tactors.size() == 1);
        _tactor = tactors[0];
    }   // end if

    if ( wasInView)
    {
        assert(_curvis);
        setVisualisation( _curvis);
        setInViewer(true);
        _allowViewUpdatedSignal = true;
        signalUpdated();
    }   // end if
}   // end forceRebuild

