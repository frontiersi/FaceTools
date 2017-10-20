#ifndef FACE_TOOLS_ACTION_SAVE_MODEL_H
#define FACE_TOOLS_ACTION_SAVE_MODEL_H

#include "FaceActionInterface.h"
#include "ModelInteractor.h"

namespace FaceTools {

class FaceTools_EXPORT ActionSaveModel : public FaceAction
{ Q_OBJECT
public:
    explicit ActionSaveModel( const std::string& iconfilename="");

    virtual const QIcon* getIcon() const { return &_icon;}
    virtual const QKeySequence* getShortcut() const { return &_keyseq;}
    virtual QString getDisplayName() const { return _dname;}

    virtual void setInteractive( ModelInteractor*, bool);

protected:
    virtual bool doAction();

private slots:
    void checkEnable();

private:
    const QIcon _icon;
    const QString _dname;
    const QKeySequence _keyseq;
    FaceModel* _fmodel;
};  // end class

}   // end namespace

#endif



