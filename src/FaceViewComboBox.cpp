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

#include <FaceViewComboBox.h>
#include <boost/filesystem.hpp>
#include <QVariant>
#include <iostream>
#include <cassert>
using FaceTools::FaceViewComboBox;
using FaceTools::FaceModel;
using FaceTools::Mint;
using FaceTools::CMint;

int FaceViewComboBox::s_mintKey(0);

namespace {
// Views (ModelInteractors) have associated with them a unique view ID.
int getMintKeyFromRow( const QComboBox* cbox, int rowi)
{
    QVariant qdata = cbox->itemData(rowi);
    bool ok = false;
    const int mintKey = qdata.toInt(&ok);
    if ( !ok)
    {
        std::cerr << "[ERROR] FaceTools::getMintKeyFromRow: failed to convert QVariant int!" << std::endl;
        assert(false);
    }   // end if
    return mintKey;
}   // end getMintKeyFromRow

void addUndoTag( const FaceModel* fmodel, std::string& mname)
{
    if ( fmodel->hasUndos())    // Denote save outstanding on this model
        mname += " (*)";
}   // end addUndoTag
}   // end namespace


// public
FaceViewComboBox::FaceViewComboBox( InteractiveModelViewer* viewer, QWidget* parent)
    : QComboBox(parent), _viewer(viewer), _curView(-1)
{
    setEditable(true);  // Allow user edits to the combo box
    setDuplicatesEnabled(false);
    setInsertPolicy( QComboBox::InsertAlphabetically);
    connect( this, SIGNAL( currentIndexChanged(int)), this, SLOT( onSelectedRow(int)));
    connect( this, SIGNAL( editTextChanged( const QString&)), this, SLOT( onEditedViewName( const QString&)));
}   // end ctor


// public
FaceViewComboBox::~FaceViewComboBox()
{
    typedef std::pair<int, Mint*> MPair;
    foreach ( const MPair& mpair, _mintLookup)
        delete mpair.second;
}   // end dtor


// public
size_t FaceViewComboBox::getModels( boost::unordered_set<FaceModel*>& fmodels) const
{
    typedef std::pair<FaceModel*, boost::unordered_set<int> > VPair;
    foreach ( const VPair& vpair, _viewKeys)
        fmodels.insert(vpair.first);
    return _viewKeys.size();
}   // end getModels


// public
const std::string& FaceViewComboBox::addView( FaceModel* fmodel, const QList<QAction*> *agroup)
{
    int vkey = s_mintKey++;     // key for this view
    _viewKeys[fmodel].insert(vkey);   // Map model to view key
    _mintLookup[vkey] = new Mint( _viewer, fmodel, agroup);

    std::string mname = createViewName( fmodel);
    _vnameLookup[vkey] = mname;
    addUndoTag( fmodel, mname);
    addItem( mname.c_str(), QVariant(vkey)); // Inserts alphabetically

    // Connect up the model for when the save filepath changes
    connect( fmodel, SIGNAL( onChangedSaveFilepath( FaceModel*)), this, SLOT( changeViewNames( FaceModel*)));
    connect( fmodel, SIGNAL( onClearedUndos( FaceModel*)), this, SLOT( changeViewNames( FaceModel*)));

    setEnabled(true);

    // Set this newly added view as the current one for interaction.
    int notused;
    const int rowi = getRowFromName( mname, notused);
    assert(notused == vkey);
    onSelectedRow(rowi);
    return _vnameLookup.at(vkey);
}   // end addView


// public
void FaceViewComboBox::removeView( const std::string& vname)
{
    int vkey;
    const int rowi = getRowFromName( vname, vkey);
    assert(rowi >= 0);

    removeItem(rowi);
   
    Mint* mint = _mintLookup.at(vkey); 
    _mintLookup.erase(vkey);
    _vnameLookup.erase(vkey);

    FaceModel* fmodel = mint->getModel();
    _viewKeys[fmodel].erase(vkey);

    if ( _viewKeys.at(fmodel).empty())
    {
        fmodel->disconnect(this);
        _viewKeys.erase(fmodel);
    }   // end if

    delete mint;
}   // end removeView


// public
size_t FaceViewComboBox::removeModel( FaceModel* fmodel)
{
    if ( _viewKeys.count(fmodel) == 0)
        return 0;

    const boost::unordered_set<int> vkeys = _viewKeys.at(fmodel);   // Copy out
    foreach ( int vkey, vkeys)
        removeView( _vnameLookup.at(vkey));

    const size_t nviews = vkeys.size();
    _viewKeys.erase(fmodel);
    return nviews;
}   // end removeModel


// public
CMint* FaceViewComboBox::getSelectedView() const
{
    int rowi = currentIndex();
    if ( rowi < 0)
        return NULL;
    return _mintLookup.at( getMintKeyFromRow( this, rowi));
}   // end getSelectedView


// public
std::string FaceViewComboBox::getSelectedViewName() const
{
    int rowi = currentIndex();
    if ( rowi < 0)
        return "";
    return _vnameLookup.at( getMintKeyFromRow( this, rowi));
}   // end getSelectedViewName


// public
size_t FaceViewComboBox::getNumModels() const
{
    return _viewKeys.size();
}   // end getNumModels


// private slot
void FaceViewComboBox::onSelectedRow( int rowi)
{
    if ( rowi < 0)
        return;
    const int vkey = getMintKeyFromRow( this, rowi);
    if ( _curView >= 0)
        _mintLookup.at(_curView)->setInteractive(false);
    _curView = vkey;
    Mint* mint = _mintLookup.at(_curView);
    mint->setInteractive(true);
    emit onViewSelected( mint->getModel(), _vnameLookup.at(vkey));
}   // end onSelectedRow


// private slot
void FaceViewComboBox::changeViewNames( FaceModel* fmodel)
{
    assert( _viewKeys.count(fmodel) > 0);

    const boost::unordered_set<int>& vkeys = _viewKeys.at(fmodel);
    foreach ( int vkey, vkeys)
    {
        const std::string& oldname = _vnameLookup.at(vkey);
        int vkeysame;
        const int rowi = getRowFromName( oldname, vkeysame);
        assert(vkeysame == vkey);
        std::string newname = createViewName( fmodel);
        _vnameLookup[vkey] = newname;
        addUndoTag( fmodel, newname);
        setItemText( rowi, newname.c_str());
    }   // end foreach
}   // end changeViewNames


// private slot
void FaceViewComboBox::onEditedViewName( const QString& text)
{
    const int rowIdx = currentIndex();
    if ( rowIdx < 0)
        return;
    Mint* mint = _mintLookup.at( getMintKeyFromRow( this, rowIdx));
    mint->getModel()->setSaveFilepath( text.toStdString()); // Causes all views related to the model to be updated
}   // end onEditedViewName


// private
int FaceViewComboBox::getRowFromName( const std::string& vname, int& vkey) const
{
    Qt::MatchFlags matchFlags = Qt::MatchStartsWith | Qt::MatchCaseSensitive;
    int rowi = findText( QString(vname.c_str()), matchFlags);
    if ( rowi >= 0)
        vkey = getMintKeyFromRow( this, rowi);
    else
        std::cerr << "[ERROR] FaceTools::FaceViewComboBox::getRowFromName: failed to find view name in combo box!" << std::endl;
    return rowi;
}   // end getRowFromName


// private
std::string FaceViewComboBox::createViewName( FaceModel* fmodel) const
{
    const std::string& fname = fmodel->getSaveFilepath();   // May not actually be saved yet if has been edited
    std::string mname = fname;
    if ( boost::filesystem::exists( boost::filesystem::path(fname)))
        mname = boost::filesystem::path(fname).filename().string();
    std::ostringstream oss;
    oss << mname << " : " << _viewKeys.at(fmodel).size();
    return oss.str();
}   // end createViewName
