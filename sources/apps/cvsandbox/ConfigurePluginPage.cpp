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

#include <PluginPropertyEditorFrame.hpp>
#include "ConfigurePluginPage.hpp"
#include "ui_ConfigurePluginPage.h"

#include <UITools.hpp>
#include <QDialog>
#include "ServiceManager.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class ConfigurePluginPagePrivate
    {
    public:
        ConfigurePluginPagePrivate( ) :
            PropertyEditor( shared_ptr<const XPluginDescriptor>( ), shared_ptr<XPlugin>( ), false, nullptr ),
            CanGoNext( false ), PluginID( )
        {

        }

        PluginPropertyEditorFrame PropertyEditor;
        bool                      CanGoNext;
        XGuid                     PluginID;
    };
}

ConfigurePluginPage::ConfigurePluginPage( bool canBeSkipped, QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::ConfigurePluginPage ),
    mData( new Private::ConfigurePluginPagePrivate( ) )
{
    ui->setupUi( this );

    mData->CanGoNext = canBeSkipped;
    ui->groupBox->layout( )->addWidget( &mData->PropertyEditor );
}

ConfigurePluginPage::~ConfigurePluginPage( )
{
    delete mData;
    delete ui;
}

// Set title of the group box
void ConfigurePluginPage::SetTitle(  const QString& title )
{
    ui->groupBox->setTitle( title );
}

// Transfer focus to property editor when the page got it
void ConfigurePluginPage::focusInEvent( QFocusEvent* event )
{
    mData->PropertyEditor.setFocus( );
    WizardPageFrame::focusInEvent( event );
}

// Check if the page is complete, so we can got the next one after applying changes of this one
bool ConfigurePluginPage::CanGoNext( ) const
{
    return mData->CanGoNext;
}

// Check if the page provides any help
bool ConfigurePluginPage::ProvidesHelp( ) const
{
    return true;
}

// Show help dialog
void ConfigurePluginPage::ShowHelp( )
{
    if ( !mData->PluginID.IsEmpty( ) )
    {
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( UITools::GetParentDialog( this ), mData->PluginID );
    }
}

// Set plug-in to configure properties of
void ConfigurePluginPage::SetPluginToConfigure( const shared_ptr<const XPluginDescriptor>& pluginDesc,
                                                const shared_ptr<XPlugin>& plugin )
{
    mData->PropertyEditor.SetPluginToConfigure( pluginDesc, plugin );
    mData->CanGoNext = true;
    mData->PluginID  = pluginDesc->ID( );
}
