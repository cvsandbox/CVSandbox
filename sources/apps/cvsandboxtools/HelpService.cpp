/*
    Common tools for Computer Vision Sandbox

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

#include "HelpService.hpp"
#include "PluginDescriptionDialog.hpp"
#include "UITools.hpp"

#include <stack>
#include <map>

#include <QEvent>
#include <QLayout>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class HelpServiceData
    {
    public:
        HelpServiceData( ) :
            ParentWidgets( ), HelpDialogs( )
        {

        }

        ~HelpServiceData( )
        {
            for ( auto& kvp : HelpDialogs )
            {
                if ( kvp.second != nullptr )
                {
                    delete kvp.second;
                }
            }

            HelpDialogs.clear( );
        }

    public:
        stack<QWidget*>                         ParentWidgets;
        map<QWidget*, PluginDescriptionDialog*> HelpDialogs;
    };
}

HelpService::HelpService( ) :
    mData( new ::Private::HelpServiceData( ) )
{

}

HelpService::~HelpService( )
{
    delete mData;
}

// Show description for the plug-in with the specified ID
void HelpService::ShowPluginDescription( QWidget* parent, const XGuid& pluginId )
{
    PluginDescriptionDialog* helpDialog = nullptr;

    // add parent to the stack
    if ( ( mData->ParentWidgets.empty( ) ) || ( mData->ParentWidgets.top( ) != parent ) )
    {
        mData->ParentWidgets.push( parent );
        connect( parent, SIGNAL(destroyed()), this, SLOT(on_parentDialog_destroyed()) );

        /*
        if ( mData->ParentWidgets.size( ) > 1 )
        {
            parent->installEventFilter( this );
        }
        */
    }

    // find help dialog for the parent widget
    auto hdIter = mData->HelpDialogs.find( parent );

    if ( ( hdIter == mData->HelpDialogs.end( ) ) || ( hdIter->second == nullptr ) )
    {
        helpDialog = new PluginDescriptionDialog( parent );
        mData->HelpDialogs[parent] = helpDialog;

        connect( helpDialog, SIGNAL(finished(int)), this, SLOT(on_helpDialog_finished(int)) );
        connect( helpDialog, SIGNAL(destroyed(QObject*)), this, SLOT(on_helpDialog_destroyed(QObject*)) );
    }
    else
    {
        helpDialog = hdIter->second;
    }

    if ( helpDialog != nullptr )
    {
        helpDialog->ShowPluginDescription( pluginId );
        helpDialog->show( );
        helpDialog->raise( );
    }
}

// Hide plug-in description window, which was opened for the specified parent
void HelpService::HidePluginDescription( QWidget* parent )
{
    auto hdIter = mData->HelpDialogs.find( parent );

    if ( hdIter != mData->HelpDialogs.end( ) )
    {
        hdIter->second->hide( );
    }
}

// Help dialog was closed (not destroyed)
void HelpService::on_helpDialog_finished( int )
{
    if ( !mData->ParentWidgets.empty( ) )
    {
        QWidget* topParent = mData->ParentWidgets.top( );
        auto     hdIter    = mData->HelpDialogs.find( topParent );

        if ( hdIter != mData->HelpDialogs.end( ) )
        {
        }
    }
}

// Help dialog was destroyed (happens on closing parent widget)
void HelpService::on_helpDialog_destroyed( QObject* object )
{
    if ( !mData->ParentWidgets.empty( ) )
    {
        QWidget* topParent = mData->ParentWidgets.top( );
        auto     hdIter    = mData->HelpDialogs.find( topParent );

        if ( ( hdIter != mData->HelpDialogs.end( ) ) && ( hdIter->second == static_cast<PluginDescriptionDialog*>( object ) ) )
        {
            hdIter->second = nullptr;
        }
    }
}

// Parent widget of some help dialog was destroyed
void HelpService::on_parentDialog_destroyed( )
{
    if ( !mData->ParentWidgets.empty( ) )
    {
        mData->HelpDialogs.erase( mData->ParentWidgets.top( ) );
        mData->ParentWidgets.pop( );
    }
}

/*
 * The concept of taking ownership and then returning Help Dialogs back did not work, so commented for now.

// Try to take ownership of the already displayed Help Dialog
void HelpService::StealPluginDescription( QWidget* parent )
{
    if ( !mData->ParentWidgets.empty( ) )
    {
        auto hdIter = mData->HelpDialogs.find( mData->ParentWidgets.top( ) );

        if ( ( hdIter != mData->HelpDialogs.end( ) ) && ( hdIter->second != nullptr ) )
        {
            PluginDescriptionDialog* helpDialog = hdIter->second;

            qDebug( "Trying to steal help window" );

            mData->ParentWidgets.push( parent );
            connect( parent, SIGNAL(destroyed()), this, SLOT(on_parentDialog_destroyed()) );
            parent->installEventFilter( this );

            mData->HelpDialogs[parent] = helpDialog;

            // set new parent for the Help Dialog
            bool isVisible = helpDialog->isVisible( );

            helpDialog->setParent( parent, helpDialog->windowFlags( ) );
            if ( isVisible )
            {
                helpDialog->show( );
            }
        }
    }
    else
    {
        qDebug( "Nothing to steal" );
    }
}

bool HelpService::eventFilter( QObject* target, QEvent* event )
{
    if ( event->type( ) == QEvent::Close )
    {
        qDebug( "got close event" );

        if ( !mData->ParentWidgets.empty( ) )
        {
            QWidget* topParent = mData->ParentWidgets.top( );
            auto     hdIter    = mData->HelpDialogs.find( topParent );

            if ( ( hdIter != mData->HelpDialogs.end( ) ) && ( hdIter->second != nullptr ) )
            {
                PluginDescriptionDialog* helpDialog = hdIter->second;

                // pop top window for now
                mData->ParentWidgets.pop( );

                // check if there is another window using same Help Dialog
                if ( !mData->ParentWidgets.empty( ) )
                {
                    QWidget* nextTopParent = mData->ParentWidgets.top( );
                    auto     hdIter2       = mData->HelpDialogs.find( nextTopParent );

                    if ( ( hdIter2 != mData->HelpDialogs.end( ) ) && ( hdIter2->second == helpDialog ) )
                    {
                        qDebug( "need to give Help Dialog back : %s", nextTopParent->windowTitle( ).toStdString( ).c_str( ) );

                        bool isVisible = helpDialog->isVisible( );

                        helpDialog->setParent( nextTopParent, helpDialog->windowFlags( ) );
                        if ( isVisible )
                        {
                            helpDialog->show( );
                        }


                        mData->HelpDialogs[topParent] = nullptr;
                    }
                }

                // push top window back
                mData->ParentWidgets.push( topParent );
            }
        }
    }

    return QObject::eventFilter( target, event );
}
*/
