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

#include "XImageExportingPlugin.hpp"

using namespace std;
using namespace CVSandbox;

XImageExportingPlugin::XImageExportingPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageExporter, ownIt ),
    mSupportedExtensions( ),
    mSupportedPixelFormats( )
{
    XErrorCode    extensionsRet     = ErrorFailed;
    xstring*      extensions        = nullptr;
    int32_t       extensionsCount   = 0;
    XErrorCode    pixelFormatsRet   = ErrorFailed;
    XPixelFormat* pixelFormats      = nullptr;
    int32_t       pixelFormatsCount = 0;

    // collect supported file extensions from a plugin
    SImageExportingPlugin* iexp = static_cast<SImageExportingPlugin*>( plugin );

    if ( iexp->GetSupportedExtensions( iexp, nullptr, &extensionsCount ) == ErrorTooSmallBuffer )
    {
        extensions = new xstring[extensionsCount];
        extensionsRet = iexp->GetSupportedExtensions( iexp, extensions, &extensionsCount );
    }

    if ( iexp->GetSupportedPixelFormats( iexp, nullptr, &pixelFormatsCount ) == ErrorTooSmallBuffer )
    {
        pixelFormats = new XPixelFormat[pixelFormatsCount];
        pixelFormatsRet = iexp->GetSupportedPixelFormats( iexp, pixelFormats, &pixelFormatsCount );
    }

    if ( extensionsRet == SuccessCode )
    {
        // copy supported extensions to vector and free xstrings
        mSupportedExtensions.reserve( extensionsCount );

        for ( int i = 0; i < extensionsCount; i++ )
        {
            mSupportedExtensions.push_back( string( extensions[i] ) );
            XStringFree( &extensions[i] );
        }
    }

    if ( pixelFormatsRet == SuccessCode )
    {
        // copy supported pixel format to vector
        mSupportedPixelFormats.reserve( pixelFormatsCount );

        for ( int i = 0; i < pixelFormatsCount; i++ )
        {
            mSupportedPixelFormats.push_back( pixelFormats[i] );
        }
    }

    // clean up
    if ( extensions != nullptr )
    {
        delete [] extensions;
    }
    if ( pixelFormats != nullptr )
    {
        delete [] pixelFormats;
    }
}

XImageExportingPlugin::~XImageExportingPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageExportingPlugin> XImageExportingPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageExportingPlugin>( new XImageExportingPlugin( plugin, ownIt ) );
}

// Get some short description of the file type
const string XImageExportingPlugin::GetFileTypeDescription( ) const
{
    string  desc;
    xstring xdesc = nullptr;

    SImageExportingPlugin* iexp = static_cast<SImageExportingPlugin*>( mPlugin );
    xdesc = iexp->GetFileTypeDescription( iexp );

    if ( xdesc != nullptr )
    {
        desc = string( xdesc );
        XStringFree( &xdesc );
    }

    return desc;
}

// Get list of supported file extensions
const vector<string> XImageExportingPlugin::GetSupportedExtensions( ) const
{
    return mSupportedExtensions;
}

// Get pixel formats supported by the exporter
const vector<XPixelFormat> XImageExportingPlugin::GetSupportedPixelFormats( ) const
{
    return mSupportedPixelFormats;
}

// Save image to the specified file
XErrorCode XImageExportingPlugin::ExportImage( const string& fileName, const shared_ptr<const XImage>& src ) const
{
    SImageExportingPlugin* iexp = static_cast<SImageExportingPlugin*>( mPlugin );
    return iexp->ExportImage( iexp, fileName.c_str( ), src->ImageData( ) );
}
