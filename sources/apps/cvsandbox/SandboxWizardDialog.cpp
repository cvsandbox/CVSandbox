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

#include <assert.h>
#include <QIcon>
#include "SandboxWizardDialog.hpp"
#include "ServiceManager.hpp"

#include "VideoProcessingWizardPage.hpp"
#include "ScriptingThreadsPage.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

namespace Private
{
    class SandboxWizardDialogData
    {
    public:
        SandboxWizardDialogData( const shared_ptr<SandboxProjectObject>& po ) :
            SandboxToEdit( po ),
            videoProcessingWizardPage( nullptr ),
            scriptingThreadsPage( nullptr )
        {
        }

        bool ApplyVideoProcessingWizardPage( );
        bool ApplyScriptingThreadsPage( );

    public:
        shared_ptr<SandboxProjectObject> SandboxToEdit;
        VideoProcessingWizardPage*       videoProcessingWizardPage;
        ScriptingThreadsPage*            scriptingThreadsPage;
    };
}

SandboxWizardDialog::SandboxWizardDialog( const shared_ptr<SandboxProjectObject>& po,
                                          QWidget* parent ) :
    PropertyPagesDialog( "SandboxWizardPropertie", parent ),
    mData( new ::Private::SandboxWizardDialogData( po ) )
{
    assert( po );

    this->setWindowTitle( "Sandbox Wizard" );
    this->setWindowIcon( QIcon( QString::fromUtf8( ":/images/icons/wizard.png" ) ) );

    // set up pages
    mData->videoProcessingWizardPage = new VideoProcessingWizardPage( this );
    mData->scriptingThreadsPage      = new ScriptingThreadsPage( this );

    // configure pages
    mData->videoProcessingWizardPage->SetCamerasToConfigure( po->GetSandboxDevices( ) );
    mData->videoProcessingWizardPage->SetCamerasProcessingGraphs( po->GetCamerasProcessingGraphs( ) );

    mData->scriptingThreadsPage->SetThreads( po->GetScriptingThreads( ) );

    // add pages to the property dialog
    AddPage( mData->videoProcessingWizardPage );
    AddPage( mData->scriptingThreadsPage );
}

SandboxWizardDialog::~SandboxWizardDialog( )
{
    delete mData;
}

// Apply currently active page
void SandboxWizardDialog::ApplyCurrentPage( int index )
{
    bool changeDetected = false;

    switch ( index )
    {
    case 0:
        changeDetected = mData->ApplyVideoProcessingWizardPage( );
        break;

    case 1:
        changeDetected = mData->ApplyScriptingThreadsPage( );
        break;

    default:
        Q_ASSERT( false );
        break;
    }

    if ( changeDetected )
    {
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->SandboxToEdit );
    }
}

// Apply all pages - OK button
void SandboxWizardDialog::ApplyAllPages( )
{
    bool changeDetected = false;

    changeDetected |= mData->ApplyVideoProcessingWizardPage( );
    changeDetected |= mData->ApplyScriptingThreadsPage( );

    if ( changeDetected )
    {
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->SandboxToEdit );
    }
}

namespace Private
{

// Apply the page configuring video processing steps
bool SandboxWizardDialogData::ApplyVideoProcessingWizardPage( )
{
    const map<XGuid, XVideoSourceProcessingGraph> processingGraps = videoProcessingWizardPage->GetCamerasProcessingGraphs( );
    bool                                          changeDetected  = false;

    if ( processingGraps != SandboxToEdit->GetCamerasProcessingGraphs( ) )
    {
        SandboxToEdit->SetCamerasProcessingGraphs( processingGraps );
        changeDetected = true;
    }

    return changeDetected;
}

// Apply the page configuring scripting threads
bool SandboxWizardDialogData::ApplyScriptingThreadsPage( )
{
    map<XGuid, ScriptingThreadDesc> threads = scriptingThreadsPage->Threads( );
    bool                            changeDetected  = false;

    if ( threads != SandboxToEdit->GetScriptingThreads( ) )
    {
        SandboxToEdit->SetScriptingThreads( threads );
        changeDetected = true;
    }

    return changeDetected;
}

} // namespace Private
