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
    struct Model
    {
        bool backfaceCulling;
        double vertexSize;
        double lineWidth;
        QColor activeColour;
        QColor surfaceColourMin;
        QColor surfaceColourMid;
        QColor surfaceColourMax;
        float minVisibleScalar;
        float maxVisibleScalar;
        int numSurfaceColours;
    };  // end struct

    struct Landmarks
    {
        double radius;
        QColor colour;
        double highlightRadius;
        QColor highlightColour;
    };  // end struct

    struct Boundary
    {
        QColor lineColour;
        double lineWidth;
        double cropFactor;  // 2.0 nominal
    };  // end struct

    Model model;
    Landmarks landmarks;
    Boundary boundary;
    std::string munits;     // Measurement units (e.g. "mm")
    QColor textColour;      // Text annotation colour
    bool showCaptions;
    double maxTriangleArea;
    double smoothFactor;
};  // end struct


}   // end namespace

#endif

