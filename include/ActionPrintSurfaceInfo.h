#ifndef FACE_TOOLS_ACTION_PRINT_SURFACE_INFO_H
#define FACE_TOOLS_ACTION_PRINT_SURFACE_INFO_H

#include "FaceActionInterface.h"    // FaceTools
#include "ModelInteractor.h"

namespace FaceTools {

class FaceTools_EXPORT ActionPrintSurfaceInfo : public FaceAction
{ Q_OBJECT
public:
    ActionPrintSurfaceInfo();

    virtual QString getDisplayName() const { return "Print Surface Info";}
    virtual void setInteractive( ModelInteractor*, bool);

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    const ModelInteractor* _interactor;
};  // end class

}   // end namespace

#endif
