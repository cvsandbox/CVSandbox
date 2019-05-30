/*
    Plug-ins' management library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
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

#include "XPluginsModule.hpp"

using namespace std;
using namespace CVSandbox;

const ModuleDescriptor XPluginsModule::BlankModuleDescriptor =
{
    { 0, 0, 0, 0 },
    { 0, 0, 0 },
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

XPluginsModule::XPluginsModule( const std::string& fileName ) :
    mModule( 0 ), mFileName( fileName ),
    mPlugins( XPluginsCollection::Create( ) ),
    // a bit of a hack, but we'll trust it since we are not going (not supposed) to change the descriptor anyway
    mDescriptor( const_cast<ModuleDescriptor*>( &BlankModuleDescriptor ) )
{
}

XPluginsModule::~XPluginsModule( )
{
    Unload( );
}

const shared_ptr<XPluginsModule> XPluginsModule::Create( const std::string& fileName )
{
    return shared_ptr<XPluginsModule>( new XPluginsModule( fileName ) );
}

// Load the module
XErrorCode XPluginsModule::Load( PluginType typesToCollect )
{
    XErrorCode retCode = SuccessCode;

    mModule = XModuleLoad( mFileName.c_str( ) );

    if ( mModule == 0 )
    {
        retCode = ErrorFailedLoadingModule;
    }
    else
    {
        // get module's API
        ModuleInitializeFunc moduleInitilizer = (ModuleInitializeFunc)
            XModuleGetSymbol( mModule, ModuleInitializeFuncName );
        GetDescriptorFunc pluginDescProvider = (GetDescriptorFunc)
            XModuleGetSymbol( mModule, GetDescriptorFuncName );

        if ( ( moduleInitilizer == 0 ) || ( pluginDescProvider == 0 ) )
        {
            // failed getting required API - wrong interface
            retCode = ErrorUnsupportedInterface;
        }
        else
        {
            ModuleDescriptor* desc = moduleInitilizer( );

            if ( desc == 0 )
            {
                return ErrorInitializationFailed;
            }
            else
            {
                mDescriptor = desc;
                mPlugins->CollectPlugins( pluginDescProvider, mDescriptor->PluginsCount, typesToCollect );
            }
        }
    }

    return retCode;
}

// Unload the module
// TODO ?: For now module can be unloaded when its plug-ins are still in use, which will cause bad
//         things to happen. Something may need to be done to avoid this from happening, if we want to
//         protect from developers affected by human factor errors.
//
void XPluginsModule::Unload( )
{
    mPlugins->Clear( );

    if ( mDescriptor != &BlankModuleDescriptor )
    {
        FreeModuleDescriptor( &mDescriptor );
        mDescriptor = const_cast<ModuleDescriptor*>( &BlankModuleDescriptor );
    }

    if ( mModule != 0 )
    {
        ModuleCleanupFunc moduleCleaner = (ModuleCleanupFunc)
            XModuleGetSymbol( mModule, ModuleCleanupFuncName );

        if ( moduleCleaner != 0 )
        {
            moduleCleaner( );
        }

        XModuleUnload( mModule );
        mModule = 0;
    }
}

const XGuid XPluginsModule::ID( ) const
{
    return XGuid( mDescriptor->ID );
}

const XVersion XPluginsModule::Version( ) const
{
    return XVersion( mDescriptor->Version );
}

const string XPluginsModule::ShortName( ) const
{
    return string( mDescriptor->ShortName );
}

const string XPluginsModule::Name( ) const
{
    return string( mDescriptor->Name );
}

const string XPluginsModule::Description( ) const
{
    return string( mDescriptor->Description );
}

const string XPluginsModule::Vendor( ) const
{
    return string( mDescriptor->Vendor );
}

const string XPluginsModule::Copyright( ) const
{
    return string( mDescriptor->Copyright );
}

const string XPluginsModule::Website( ) const
{
    return string( mDescriptor->Website );
}

const shared_ptr<const XImage> XPluginsModule::Icon( bool getSmall ) const
{
    return XImage::Create( const_cast<const ximage*>(
        ( getSmall ) ? mDescriptor->SmallIcon : mDescriptor->Icon ) );
}

// Get plug-in descriptor by its short name
const shared_ptr<const XPluginDescriptor> XPluginsModule::GetPlugin( const string& shortName ) const
{
    return mPlugins->GetPlugin( shortName );
}

// Get plug-in descriptor by its ID
const shared_ptr<const XPluginDescriptor> XPluginsModule::GetPlugin( const XGuid& id ) const
{
    return mPlugins->GetPlugin( id );
}

// Get collection of this module's plug-ins
const shared_ptr<const XPluginsCollection> XPluginsModule::GetPlugins( ) const
{
    return mPlugins;
}

// Get collection of plug-ins of the specified type
const shared_ptr<const XPluginsCollection> XPluginsModule::GetPluginsOfType( PluginType typesMask ) const
{
    return mPlugins->GetPluginsOfType( typesMask );
}

// Number of plug-in loaded for the module (the module may have more than that)
size_t XPluginsModule::Count( ) const
{
    return mPlugins->Count( );
}

// Number of plug-in descriptors of the specified type
size_t XPluginsModule::CountType( PluginType typeMask ) const
{
    return mPlugins->CountType( typeMask );
}
