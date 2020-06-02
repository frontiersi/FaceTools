/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
#include <Action/FaceAction.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;


QString FaceTools::toSexString( int8_t s)
{
    if ( s == FEMALE_SEX)
        return "F";
    else if ( s == MALE_SEX)
        return "M";
    return "F M";
}   // end toSexString


QString FaceTools::toLongSexString( int8_t s)
{
    if ( s == FEMALE_SEX)
        return "Female";
    else if ( s == MALE_SEX)
        return "Male";
    return "Female | Male";
}   // end namespace


int8_t FaceTools::fromSexString( const QString& s)
{
    const QString ss = s.toLower().trimmed();
    if ( ss == "f" || ss == "female")
        return FEMALE_SEX;
    if ( ss == "m" || ss == "male")
        return MALE_SEX;
    return UNKNOWN_SEX;
}   // end fromSexString


Event FaceTools::Action::operator|( const Event &e0, const Event &e1)
{
    return Event( uint32_t(e0) | uint32_t(e1));
}   // end operator|


void FaceTools::Action::operator|=( Event &e, const Event &e1) { e = operator|( e, e1);}
Event& FaceTools::Action::add( Event &e, const Event &e1) { e = operator|( e, e1); return e;}
Event& FaceTools::Action::remove( Event &e, const Event &e1) { e = Event( uint32_t(e) & ~uint32_t(e1)); return e;}
Event FaceTools::Action::operator&( const Event &e0, const Event &e1) { return Event( uint32_t(e0) & uint32_t(e1));}
bool FaceTools::Action::has( const Event &e0, const Event &e1) { return operator&( e0, e1) != Event::NONE;}


std::ostream &FaceTools::Action::operator<<( std::ostream &os, const Event &e)
{
    std::vector<std::string> nms;

    if ( has( e, Event::USER))
        nms.push_back("USER");
    if ( has( e, Event::LOADED_MODEL))
        nms.push_back("LOADED_MODEL");
    if ( has( e, Event::SAVED_MODEL))
        nms.push_back("SAVED_MODEL");
    if ( has( e, Event::CLOSED_MODEL))
        nms.push_back("CLOSED_MODEL");
    if ( has( e, Event::MESH_CHANGE))
        nms.push_back("MESH_CHANGE");
    if ( has( e, Event::CONNECTIVITY_CHANGE))
        nms.push_back("CONNECTIVITY_CHANGE");
    if ( has( e, Event::AFFINE_CHANGE))
        nms.push_back("AFFINE_CHANGE");
    if ( has( e, Event::MASK_CHANGE))
        nms.push_back("MASK_CHANGE");
    if ( has( e, Event::SURFACE_DATA_CHANGE))
        nms.push_back("SURFACE_DATA_CHANGE");
    if ( has( e, Event::LANDMARKS_CHANGE))
        nms.push_back("LANDMARKS_CHANGE");
    if ( has( e, Event::METRICS_CHANGE))
        nms.push_back("METRICS_CHANGE");
    if ( has( e, Event::STATS_CHANGE))
        nms.push_back("STATS_CHANGE");

    if ( has( e, Event::PATHS_CHANGE))
        nms.push_back("PATHS_CHANGE");
    if ( has( e, Event::VIEW_CHANGE))
        nms.push_back("VIEW_CHANGE");
    if ( has( e, Event::VIEWER_CHANGE))
        nms.push_back("VIEWER_CHANGE");
    if ( has( e, Event::CAMERA_CHANGE))
        nms.push_back("CAMERA_CHANGE");

    if ( has( e, Event::ACTOR_MOVE))
        nms.push_back("ACTOR_MOVE");
    if ( has( e, Event::ASSESSMENT_CHANGE))
        nms.push_back("ASSESSMENT_CHANGE");

    if ( has( e, Event::RESTORE_CHANGE))
        nms.push_back("RESTORE_CHANGE");
    if ( has( e, Event::ALL_VIEWS))
        nms.push_back("ALL_VIEWS");
    if ( has( e, Event::ALL_VIEWERS))
        nms.push_back("ALL_VIEWERS");
    if ( has( e, Event::MODEL_SELECT))
        nms.push_back("MODEL_SELECT");
    if ( has( e, Event::START_MOVE))
        nms.push_back("START_MOVE");
    if ( has( e, Event::FINISH_MOVE))
        nms.push_back("FINISH_MOVE");
    if ( has( e, Event::CANCELLED))
        nms.push_back("CANCELLED");
    if ( has( e, Event::ERR))
        nms.push_back("ERR");

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
    return os << nm;
}   // end operator<<
