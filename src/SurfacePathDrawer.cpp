/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <SurfacePathDrawer.h>
#include <InteractiveModelViewer.h>
#include <FaceView.h>
#include <FaceModel.h>
#include <ObjMetaData.h>
#include <VtkTools.h>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
#include <DijkstraShortestPathFinder.h> // RFeatures
using FaceTools::SurfacePathDrawer;
using FaceTools::FaceControl;
using FaceTools::ObjMetaData;
using FaceTools::InteractiveModelViewer;
using RFeatures::ObjModel;


// public
SurfacePathDrawer::SurfacePathDrawer( FaceControl* fcont, const std::string& munits, const QColor& tcol, bool showCaps)
    : _fcont(fcont), _h0(NULL), _h1(NULL), _munits(munits), _visible(false), _pathID(-1),
      _lenText( vtkSmartPointer<vtkTextActor>::New()),
      _lenCaption( vtkSmartPointer<vtkCaptionActor2D>::New())
{
    _lenText->GetTextProperty()->SetJustificationToRight();
    _lenText->GetTextProperty()->SetFontFamilyToArial();
    _lenText->GetTextProperty()->SetFontSize(16);
    _lenText->GetTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());

    _lenCaption->BorderOff();
    _lenCaption->GetCaptionTextProperty()->BoldOff();
    _lenCaption->GetCaptionTextProperty()->ItalicOff();
    _lenCaption->GetCaptionTextProperty()->ShadowOff();
    _lenCaption->GetCaptionTextProperty()->SetFontFamilyToArial();
    _lenCaption->GetCaptionTextProperty()->SetFontSize(8);
    _lenCaption->GetCaptionTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
    _lenCaption->SetVisibility( showCaps);
    _lenCaption->SetPickable( true);
}   // end ctor


// public
SurfacePathDrawer::~SurfacePathDrawer()
{
    removePath();
    clearHandles();
}   // end dtor


// public
void SurfacePathDrawer::clearHandles()
{
    if ( _h0)
    {
        delete _h0;
        _h0 = NULL;
    }   // end if
    if ( _h1)
    {
        delete _h1;
        _h1 = NULL;
    }   // end if
}   // end clearHandles


// public
size_t SurfacePathDrawer::nhandles() const
{
    size_t n = 0;
    if ( _h0)
        n++;
    if ( _h1)
        n++;
    return n;
}   // end nhandles


void SurfacePathDrawer::setUnits( const std::string& units) { _munits = units;}
bool SurfacePathDrawer::isVisible() const { return _visible;}


// public
void SurfacePathDrawer::setVisible( bool enable)
{
    removePath();
    _visible = false;
    if ( enable)
    {
        addPath();
        _visible = true;
    }   // end if
}   // end setVisible


// public
SurfacePathDrawer::Handle* SurfacePathDrawer::setHandle0( const QPoint& p)
{
    if ( !_h0)
        _h0 = new Handle(this);
    _h0->set(p);
    return _h0;
}   // end setHandle0


// public
SurfacePathDrawer::Handle* SurfacePathDrawer::setHandle1( const QPoint& p)
{
    if ( !_h1)
        _h1 = new Handle(this);
    _h1->set(p);
    return _h1;
}   // end setHandle1


// public
void SurfacePathDrawer::highlightHandle0( bool v)
{
    if ( _h0)
        _h0->highlight(v);
}   // end highlighHandle0


// public
void SurfacePathDrawer::highlightHandle1( bool v)
{
    if ( _h1)
        _h1->highlight(v);
}   // end highlighHandle1


// public
SurfacePathDrawer::Handle* SurfacePathDrawer::handle( const QPoint& p) const
{
    Handle* handle = NULL;
    FaceTools::ModelViewer* viewer = _fcont->getViewer();
    const vtkProp* prop = viewer->getPointedAt(p);
    if ( _h0 && prop == _h0->_actor)
        handle = _h0;
    else if ( _h1 && prop == _h1->_actor)
        handle = _h1;
    return handle;
}   // end handle


// private
void SurfacePathDrawer::removePath()
{
    InteractiveModelViewer* viewer = _fcont->getViewer();
    if ( _h0)
    {
        viewer->remove(_h0->_actor);
        _fcont->removeTempMemberProp( _h0->_actor);
    }   // end if
    if ( _h1)
    {
        viewer->remove(_h1->_actor);
        _fcont->removeTempMemberProp( _h1->_actor);
    }   // end if
    if ( _pathID >= 0)
        viewer->remove(_pathID);
    viewer->remove(_lenText);
    viewer->remove(_lenCaption);
    _fcont->removeTempMemberProp( _lenCaption);
    _pathID = -1;
}   // end removePath


// private
void SurfacePathDrawer::addPath()
{
    if ( _h0 && _h1)
    {
        std::vector<cv::Vec3f> lpath;
        if ( createPath( lpath))
        {
            const ModelViewer::VisOptions loptions( 1.0f,1.0f,1.0f,1.0f,false,1.0f,5.0f);
            InteractiveModelViewer* viewer = _fcont->getViewer();
            _pathID = viewer->addLine( lpath, false, loptions);
            viewer->add(_h0->_actor);
            viewer->add(_h1->_actor);
            viewer->add(_lenText);
            viewer->add(_lenCaption);
            _fcont->addTempMemberProp( _h0->_actor);
            _fcont->addTempMemberProp( _h1->_actor);
            _fcont->addTempMemberProp( _lenCaption);    // Need to add the caption too since it can occlude the handles!
        }   // end if
    }   // end if
}   // end addPath


SurfacePathDrawer::Handle::Handle( SurfacePathDrawer* spd)
    : _host(spd)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    _source = vtkSmartPointer<vtkSphereSource>::New();
    _actor = vtkSmartPointer<vtkActor>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);
    highlight(false);
}   // end ctor


// public
void SurfacePathDrawer::Handle::set( const QPoint& p)
{
    cv::Vec3f wpos;
    const vtkActor* prop = _host->_fcont->getView()->getSurfaceActor();
    _host->_fcont->getViewer()->calcSurfacePosition( prop, p, wpos);
    _source->SetCenter( wpos[0], wpos[1], wpos[2]);
    _source->Update();
    _host->removePath();
    _host->addPath();
}   // end set


// public
cv::Vec3f SurfacePathDrawer::Handle::get() const
{
    const double *c = _source->GetCenter();
    return cv::Vec3f( (float)c[0], (float)c[1], (float)c[2]);
}   // end get


// public
void SurfacePathDrawer::Handle::highlight( bool v)
{
    double opacity = 1.0;
    double rad = 2.0;
    double cols[3] = {0.8,0.4,1.0};
    if (v)
    {
        cols[0] = 0.5;
        cols[1] = 0.1;
        cols[2] = 0.7;
        rad = 2.2;
        opacity = 0.5;
    }   // end if
    _actor->GetProperty()->SetColor( cols[0], cols[1], cols[2]);
    _actor->GetProperty()->SetOpacity( opacity);
    _source->SetRadius( rad);
}   // end highlight


// private
bool SurfacePathDrawer::createPath( std::vector<cv::Vec3f>& lpath)
{
    if ( nhandles() < 2)
        return false;

    const ObjMetaData::Ptr omd = _fcont->getModel()->getObjectMeta();
    const ObjModel::Ptr model = omd->getObject();
    assert( omd->getKDTree());

    // Find shortest path over the surface (NOT GEODESIC!)
    RFeatures::DijkstraShortestPathFinder dspf( model);
    dspf.setEndPointVertexIndices( omd->getKDTree()->find( _h0->get()), omd->getKDTree()->find( _h1->get()));
    std::vector<int> pvids;
    dspf.findShortestPath( pvids);
    if ( pvids.size() < 1)
        return false;

    // Sum over the actual length of the surface path.
    cv::Vec3f pv = model->vtx(pvids[0]);
    double psum = 0.0;  // Surface length
    BOOST_FOREACH ( int vidx, pvids)
    {
        const cv::Vec3f& v = model->vtx(vidx);
        psum += cv::norm( v - pv);
        pv = v;
        lpath.push_back(v);
    }   // end foreach
    const double elen = cv::norm( model->vtx(pvids.front()), model->vtx(pvids.back())); // Euclidean length

    // Set the caption's attachment point in the middle of the path
    const cv::Vec3f& av = model->vtx(pvids[pvids.size()/2]);
    double attachPoint[3] = { av[0], av[1], av[2]};
    _lenCaption->SetAttachmentPoint( attachPoint);

    // Set the text contents for the label and the caption
    std::ostringstream oss1, oss2;
    oss1 << "Caliper: " << std::setw(5) << std::fixed << std::setprecision(1) << elen << " " << _munits << std::endl;
    oss2 << "Geodesic: " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " " << _munits;
    _lenText->SetInput( (oss1.str() + oss2.str()).c_str());
    _lenText->SetDisplayPosition( (int)_fcont->getViewer()->getWidth() - 10, 10);  // Bottom right

    // Stick a caption actor at the finishing point
    std::ostringstream caposs;
    caposs << std::fixed << std::setprecision(1) << elen << " " << _munits;
    _lenCaption->SetCaption( caposs.str().c_str());
    return true;
}   // end createPath


