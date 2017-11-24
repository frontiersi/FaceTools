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
FaceViewComboBox::FaceViewComboBox( QWidget* parent)
    : QComboBox(parent)
{
    //setEditable(true);  // Allow user edits to the combo box
    setEditable(false);
    setDuplicatesEnabled(false);
    setInsertPolicy( QComboBox::InsertAlphabetically);
    connect( this, SIGNAL( currentIndexChanged(int)), this, SLOT( onSelectedRow(int)));
    connect( this, SIGNAL( editTextChanged( const QString&)), this, SLOT( onEditedViewName( const QString&)));
}   // end ctor


// public
void FaceViewComboBox::addView( Mint* mint)
{
    int vkey = s_mintKey++;     // key for this view
    _viewKeys[fmodel].insert(vkey);   // Map model to view key
    _mintLookup[vkey] = mint;
    _mintLookupR[mint] = vkey;

    std::string mname = createViewName( mint->getView()->getModel());
    _vnameLookup[vkey] = mname;
    addUndoTag( fmodel, mname);
    addItem( mname.c_str(), QVariant(vkey)); // Inserts alphabetically

    setEnabled(true);

    // Set this newly added view as the current one for interaction.
    int notused;
    const int rowi = getRowFromName( mname, notused);
    assert(notused == vkey);
    onSelectedRow(rowi);
}   // end addView


// public
void FaceViewComboBox::removeView( Mint* mint)
{
    assert(mint);
    int vkey = _mintLookupR.at(mint);
    int rowi = getRowFromName( _vnameLookup.at(vkey), vkey);
    removeItem(rowi);

    mint->disconnect(this);
    _mintLookup.erase(vkey);
    _mintLookupR.erase(mint);
    _vnameLookup.erase(vkey);

    FaceModel* fmodel = mint->getView()->getModel();
    _viewKeys[fmodel].erase(vkey);
    if ( _viewKeys.at(fmodel).empty())
        _viewKeys.erase(fmodel);
}   // end removeView


// public
Mint* FaceViewComboBox::getActive( std::string *vname)
{
    int rowi = currentIndex();
    if ( rowi < 0)
        return NULL;
    int vkey = getMintKeyFromRow( this, rowi);
    Mint* mint = _mintLookup.at( vkey);
    if ( vname)
        *vname = _vnameLookup.at( vkey);
    return mint;
}   // end getActive


// private slot
void FaceViewComboBox::onSelectedRow( int rowi)
{
    if ( rowi < 0)
        return;
    const int vkey = getMintKeyFromRow( this, rowi);
    Mint* mint = _mintLookup.at(vkey);
    emit onActivated( mint);
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
    mint->getView()->getModel()->setSaveFilepath( text.toStdString()); // Causes all views related to the model to be updated
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
