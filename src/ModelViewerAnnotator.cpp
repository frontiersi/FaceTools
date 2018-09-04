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

#include <ModelViewerAnnotator.h>
#include <VtkTools.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
#include <algorithm>
using FaceTools::ModelViewerAnnotator;

struct ModelViewerAnnotator::Message
{
    Message( float cp, float rp, const std::string& txt, ModelViewerAnnotator::TextJustification justification, vtkRenderer* r)
        : colProp(cp), rowProp(rp), msg( vtkSmartPointer<vtkTextActor>::New()), ren(r)
    {
        switch (justification)
        {
            case ModelViewerAnnotator::LeftJustify:
                msg->GetTextProperty()->SetJustificationToLeft();
                break;
            case ModelViewerAnnotator::RightJustify:
                msg->GetTextProperty()->SetJustificationToRight();
                break;
            default:
                msg->GetTextProperty()->SetJustificationToCentered();
        }   // end switch
        msg->GetTextProperty()->SetFontFamilyToTimes();
        msg->GetTextProperty()->SetFontSize(17);
        msg->GetTextProperty()->SetColor( 1, 1, 1);
        msg->SetInput( txt.c_str());
        ren->AddActor2D(msg);
    }   // end ctor

    ~Message()
    {
        ren->RemoveActor2D(msg);
    }   // end dtor

    void setInDisplay()
    {
        const int col = (int)(ren->GetSize()[0] * colProp);
        const int row = (int)(ren->GetSize()[1] * rowProp);
        msg->SetDisplayPosition( col, row);
    }   // end setInDisplay

    float colProp;
    float rowProp;
    vtkSmartPointer<vtkTextActor> msg;
    vtkRenderer* ren;
};  // end struct


// public
ModelViewerAnnotator::ModelViewerAnnotator( vtkRenderer* ren)
    : _renderer(ren)
{}   // end ctor


// public
ModelViewerAnnotator::~ModelViewerAnnotator()
{
    std::for_each(std::begin(_messages), std::end(_messages), []( const auto& mp){ delete mp.second;});
    _messages.clear();
}   // end dtor


// public
int ModelViewerAnnotator::showMessage( float col, float row, TextJustification justification, const std::string& txt)
{
    static int messageID = 0;
    _messages[messageID] = new Message( col, row, txt, justification, _renderer);
    _messages[messageID]->setInDisplay();
    messageID++;
    return messageID-1;
}   // end showMessage


// public
bool ModelViewerAnnotator::removeMessage( int msgID)
{
    if ( _messages.count(msgID) == 0)
        return false;
    delete _messages.at(msgID);
    _messages.erase(msgID);
    return true;
}   // end removeMessage


// public
void ModelViewerAnnotator::doOnUpdateMessagePositions()
{
    std::for_each(std::begin(_messages), std::end(_messages), []( auto& m){ m.second->setInDisplay();});
}   // end doOnUpdateMessagePositions

