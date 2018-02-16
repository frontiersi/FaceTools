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

#ifndef FACE_TOOLS_MODEL_OPTIONS_H
#define FACE_TOOLS_MODEL_OPTIONS_H

#include "FaceTools_Export.h"
#include <QColor>
#include <string>

namespace FaceTools
{

struct FaceTools_EXPORT ModelOptions
{
    ModelOptions()
    {
        model.backfaceCulling = false;
        model.vertexSize = 1.0;
        model.lineWidth = 1.0;
        model.opacity = 1.0;
        model.numSurfaceColours = 100;
        landmarks.radius = 1.7;
        boundary.cropFactor = 2.0;
        showAxes = false;
        showCaptions = true;
        maxTriangleArea = 2.0;
        smoothFactor = 0.8;
    }   // end ctor

    struct Model
    {
        bool backfaceCulling;
        double vertexSize;
        double lineWidth;
        double opacity;
        QColor surfaceColourMin;
        QColor surfaceColourMid;
        QColor surfaceColourMax;
        int numSurfaceColours;
        float minVisibleScalar;
        float maxVisibleScalar;
    };  // end struct

    struct Landmarks
    {
        double radius;
        QColor colour;
    };  // end struct

    struct Boundary
    {
        double cropFactor;  // 2.0 nominal
    };  // end struct

    Model model;
    Landmarks landmarks;
    Boundary boundary;
    std::string munits;     // Measurement units (e.g. "mm")
    QColor textColour;      // Text annotation colour
    bool showAxes;
    bool showCaptions;
    double maxEdgeLength;
    double smoothFactor;    // Not used
};  // end struct


}   // end namespace

#endif

