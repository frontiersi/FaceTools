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

#include <FaceTypes.h>
#include <FaceAction.h>
using FaceTools::Action::EventGroup;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;


QString FaceTools::toSexString( int8_t s)
{
    if ( s == FaceTools::UNKNOWN_SEX)
        return "U";

    std::ostringstream oss;
    if ( s & FaceTools::FEMALE_SEX)
        oss << "F ";
    if ( s & FaceTools::MALE_SEX)
        oss << "M";

    QString sstr = oss.str().c_str();
    return sstr.trimmed();
}   // end toSexString


QString FaceTools::toLongSexString( int8_t s)
{
    if ( s == FaceTools::UNKNOWN_SEX)
        return "Unknown";

    QString fstr, mstr, estr;
    if ( s & FaceTools::FEMALE_SEX)
        fstr = "Female";
    if ( s & FaceTools::MALE_SEX)
        mstr = "Male";
    if ( !mstr.isEmpty() && !fstr.isEmpty())
        estr = " / ";
    return fstr + estr + mstr;
}   // end namespace


int8_t FaceTools::fromSexString( const QString& s)
{
    int8_t sex = FaceTools::UNKNOWN_SEX;
    QStringList toks = s.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    for ( const QString& t : toks)
    {
        QString lt = t.toLower();
        if ( lt == "f")
            sex |= FaceTools::FEMALE_SEX;
        else if ( lt == "m")
            sex |= FaceTools::MALE_SEX;
    }   // end for

    return sex;
}   // end fromSexString


int8_t FaceTools::fromLongSexString( const QString& s)
{
    int8_t sex = FaceTools::UNKNOWN_SEX;
    QStringList toks = s.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    for ( const QString& t : toks)
    {
        QString lt = t.toLower();
        if ( lt == "female")
            sex |= FaceTools::FEMALE_SEX;
        else if ( lt == "male")
            sex |= FaceTools::MALE_SEX;
    }   // end for

    return sex;
}   // end fromLongSexString


EventGroup::EventGroup() : _E(Event::NONE) {}

EventGroup::EventGroup( Event E) : _E(E)
{
}   // end ctor

EventGroup::EventGroup( Event e0, Event e1) : _E(e0)
{
    add( e1);
}   // end ctor

EventGroup::EventGroup( Event e0, Event e1, Event e2) : _E(e0)
{
    add({e1, e2});
}   // end ctor

EventGroup::EventGroup( Event e0, Event e1, Event e2, Event e3) : _E(e0)
{
    add({e1, e2, e3});
}   // end ctor

EventGroup::EventGroup( Event e0, Event e1, Event e2, Event e3, Event e4) : _E(e0)
{
    add({e1, e2, e3, e4});
}   // end ctor

EventGroup::~EventGroup(){}

bool EventGroup::has( EventGroup e) const { return (int(e.event()) & int(_E)) > 0;}

bool EventGroup::is( EventGroup e) const { return _E == e.event();}

Event EventGroup::add( EventGroup ec) { return (_E = Event(int(_E) | int(ec.event())));}


std::string EventGroup::name() const
{
    std::vector<std::string> nms;

    if ( has(Event::ACT_CANCELLED))
        nms.push_back("ACT_CANCELLED");
    if ( has(Event::ACT_COMPLETE))
        nms.push_back("ACT_COMPLETE");
    if ( has(Event::MODEL_SELECT))
        nms.push_back("MODEL_SELECT");
    if ( has(Event::USER))
        nms.push_back("USER");
    if ( has(Event::LOADED_MODEL))
        nms.push_back("LOADED_MODEL");
    if ( has(Event::SAVED_MODEL))
        nms.push_back("SAVED_MODEL");
    if ( has(Event::CLOSED_MODEL))
        nms.push_back("CLOSED_MODEL");
    if ( has(Event::FACE_DETECTED))
        nms.push_back("FACE_DETECTED");
    if ( has(Event::GEOMETRY_CHANGE))
        nms.push_back("GEOMETRY_CHANGE");
    if ( has(Event::CONNECTIVITY_CHANGE))
        nms.push_back("CONNECTIVITY_CHANGE");
    if ( has(Event::AFFINE_CHANGE))
        nms.push_back("AFFINE_CHANGE");
    if ( has(Event::ORIENTATION_CHANGE))
        nms.push_back("ORIENTATION_CHANGE");
    if ( has(Event::SURFACE_DATA_CHANGE))
        nms.push_back("SURFACE_DATA_CHANGE");
    if ( has(Event::LANDMARKS_CHANGE))
        nms.push_back("LANDMARKS_CHANGE");
    if ( has(Event::METRICS_CHANGE))
        nms.push_back("METRICS_CHANGE");
    if ( has(Event::STATISTICS_CHANGE))
        nms.push_back("STATISTICS_CHANGE");
    if ( has(Event::PATHS_CHANGE))
        nms.push_back("PATHS_CHANGE");
    if ( has(Event::VIEW_CHANGE))
        nms.push_back("VIEW_CHANGE");
    if ( has(Event::VIEWER_CHANGE))
        nms.push_back("VIEWER_CHANGE");
    if ( has(Event::CAMERA_CHANGE))
        nms.push_back("CAMERA_CHANGE");
    if ( has(Event::ACTOR_MOVE))
        nms.push_back("ACTOR_MOVE");
    if ( has(Event::REPORT_CREATED))
        nms.push_back("REPORT_CREATED");
    if ( has(Event::METADATA_CHANGE))
        nms.push_back("METADATA_CHANGE");
    if ( has(Event::ASSESSMENT_CHANGE))
        nms.push_back("ASSESSMENT_CHANGE");
    if ( has(Event::U3D_MODEL_CHANGE))
        nms.push_back("U3D_MODEL_CHANGE");
    if ( has(Event::ALL_VIEWS))
        nms.push_back("ALL_VIEWS");
    if ( has(Event::ALL_VIEWERS))
        nms.push_back("ALL_VIEWERS");

    std::string nm = "[ ";
    if ( nms.empty())
        nm += "NONE";
    else
    {
        nm += nms[0];
        for ( size_t i = 1; i < nms.size(); ++i)
            nm += " | " + nms[i];
    }   // end else

    nm += " ]";
    return nm;
}   // end name

/*
bool EventGroup::operator()( const std::function<bool(Event)>& checkEvent)
{
    if ( has(Event::ACT_CANCELLED))
        checkEvent(Event::ACT_CANCELLED);
    if ( has(Event::ACT_COMPLETE))
        checkEvent(Event::ACT_COMPLETE);
    if ( has(Event::MODEL_SELECT))
        checkEvent(Event::MODEL_SELECT);
    if ( has(Event::LOADED_MODEL))
        checkEvent(Event::LOADED_MODEL);
    if ( has(Event::SAVED_MODEL))
        checkEvent(Event::SAVED_MODEL);
    if ( has(Event::CLOSED_MODEL))
        checkEvent(Event::CLOSED_MODEL);
    if ( has(Event::FACE_DETECTED))
        checkEvent(Event::FACE_DETECTED);
    if ( has(Event::GEOMETRY_CHANGE))
        checkEvent(Event::GEOMETRY_CHANGE);
    if ( has(Event::CONNECTIVITY_CHANGE))
        checkEvent(Event::CONNECTIVITY_CHANGE);
    if ( has(Event::AFFINE_CHANGE))
        checkEvent(Event::AFFINE_CHANGE);
    if ( has(Event::ORIENTATION_CHANGE))
        checkEvent(Event::ORIENTATION_CHANGE);
    if ( has(Event::SURFACE_DATA_CHANGE))
        checkEvent(Event::SURFACE_DATA_CHANGE);
    if ( has(Event::LANDMARKS_CHANGE))
        checkEvent(Event::LANDMARKS_CHANGE);
    if ( has(Event::METRICS_CHANGE))
        checkEvent(Event::METRICS_CHANGE);
    if ( has(Event::STATISTICS_CHANGE))
        checkEvent(Event::STATISTICS_CHANGE);
    if ( has(Event::PATHS_CHANGE))
        checkEvent(Event::PATHS_CHANGE);
    if ( has(Event::VIEW_CHANGE))
        checkEvent(Event::VIEW_CHANGE);
    if ( has(Event::VIEWER_CHANGE))
        checkEvent(Event::VIEWER_CHANGE);
    if ( has(Event::CAMERA_CHANGE))
        checkEvent(Event::CAMERA_CHANGE);
    if ( has(Event::ACTOR_MOVE))
        checkEvent(Event::ACTOR_MOVE);
    if ( has(Event::REPORT_CREATED))
        checkEvent(Event::REPORT_CREATED);
    if ( has(Event::METADATA_CHANGE))
        checkEvent(Event::METADATA_CHANGE);
    if ( has(Event::ASSESSMENT_CHANGE))
        checkEvent(Event::ASSESSMENT_CHANGE);
    if ( has(Event::U3D_MODEL_CHANGE))
        checkEvent(Event::U3D_MODEL_CHANGE);
    if ( has(Event::ALL_VIEWS))
        checkEvent(Event::ALL_VIEWS);
    if ( has(Event::ALL_VIEWERS))
        checkEvent(Event::ALL_VIEWERS);
}   // end operator()
*/
