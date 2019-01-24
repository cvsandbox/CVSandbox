/*
    Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
    http://www.cvsandbox.com/contacts.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <ErrorProviderHelper.hpp>
#include "ProjectObjectNamePage.hpp"
#include "ui_ProjectObjectNamePage.h"
#include "ServiceManager.hpp"

using namespace CVSandbox;
using namespace std;

namespace Private
{
    class ProjectObjectNamePageData
    {
    public:
        ProjectObjectNamePageData( ) :
            ErrorProvider( ), CanGoNext( false ), ParentId( ), ObjectName( ), ObjectTypeName( "Object" )
        {
        }

    public:
        ErrorProviderHelper ErrorProvider;
        bool    CanGoNext;
        XGuid   ParentId;
        string  ObjectName;
        QString ObjectTypeName;
    };
}

ProjectObjectNamePage::ProjectObjectNamePage( QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::ProjectObjectNamePage ),
    mData( new Private::ProjectObjectNamePageData( ) )
{
    ui->setupUi( this );
    mData->ErrorProvider.SetLabel( ui->errorLabel );
}

ProjectObjectNamePage::~ProjectObjectNamePage( )
{
    delete mData;
    delete ui;
}

// Set properties of the object to edit
void ProjectObjectNamePage::SetObjectProperties( const XGuid& parentId,
                                                 const string& name,
                                                 const string& description )
{
    mData->ParentId = parentId;
    mData->ObjectName = name;

    // set object name
    if ( !name.empty( ) )
    {
        ui->nameEditBox->setText( QString::fromUtf8( name.c_str( ) ) );
        ui->descriptionEditBox->setPlainText( QString::fromUtf8( description.c_str( ) ) );
    }

    ValidateObjectName( );
}

// Set icon displayed next to object's name entry
void ProjectObjectNamePage::SetObjectIcon( const QString& iconResource )
{
    ui->objectIcon->setPixmap( QPixmap( iconResource ) );
}

// Set the name of object type
void ProjectObjectNamePage::SetObjectTypeName( const QString& name )
{
    mData->ObjectTypeName = name;
    ui->groupBox->setTitle( name + " na&me" );
    ValidateObjectName( );
}

// Handle focus event
void ProjectObjectNamePage::focusInEvent( QFocusEvent* event )
{
    ui->nameEditBox->setFocus( );
    WizardPageFrame::focusInEvent( event );
}

// Check if the page is complete, so we can got the next one after applying changes of this one
bool ProjectObjectNamePage::CanGoNext( ) const
{
    return mData->CanGoNext;
}

// Check if the specified object's name is valid
void ProjectObjectNamePage::ValidateObjectName( )
{
    QString name    = ui->nameEditBox->text( ).trimmed( );
    bool    isValid = false;

    // check if name is empty or not
    if ( name.isEmpty( ) )
    {
        mData->ErrorProvider.SetError( mData->ObjectTypeName + "'s name cannot be empty.", ui->nameEditBox );
    }
    else
    {
        mData->ErrorProvider.ClearError( );
        isValid = true;
    }

    if ( isValid )
    {
        string stdName( name.toUtf8( ).data( ) );

        if ( stdName != mData->ObjectName )
        {
            shared_ptr<const IProjectManager> projectManager = ServiceManager::Instance( ).GetProjectManager( );

            // check if another object exists with such name
            const shared_ptr<const ProjectObject> po = projectManager->FindChildObject( mData->ParentId, stdName );

            if ( po )
            {
                mData->ErrorProvider.SetError( "Another object exists with the specified name.", ui->nameEditBox );
                isValid = false;
            }

            // check for valid characters
            if ( ( isValid ) && ( !projectManager->CheckIfObjectNameIsValid( stdName ) ) )
            {
                mData->ErrorProvider.SetError( "Invalid character found in the object name.", ui->nameEditBox );
                isValid = false;
            }
        }
    }

    if ( mData->CanGoNext != isValid )
    {
        mData->CanGoNext = isValid;
        emit CompleteStatusChanged( mData->CanGoNext );
    }
}

// Object's name has changed, so need to validate it
void ProjectObjectNamePage::on_nameEditBox_textChanged( const QString& )
{
    ValidateObjectName( );
}

// Get name of the object to add/update
const string ProjectObjectNamePage::GetObjectName( ) const
{
    return string( ui->nameEditBox->text( ).trimmed( ).toUtf8( ) );
}

// Get description of the object to add/update
const string ProjectObjectNamePage::GetObjectDescription( ) const
{
    return string( ui->descriptionEditBox->toPlainText( ).toUtf8( ) );
}
