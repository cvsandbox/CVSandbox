/*
    Plug-ins' management library of Computer Vision Sandbox

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

#include "XImageImportingPlugin.hpp"
#include <assert.h>

using namespace std;
using namespace CVSandbox;

XImageImportingPlugin::XImageImportingPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageImporter, ownIt ),
    mSupportedExtensions( )
{
    XErrorCode ret        = ErrorFailed;
    xstring*   extensions = nullptr;
    int32_t    count      = 0;

    // collect supported file extensions from a plugin
    SImageImportingPlugin* iimp = static_cast<SImageImportingPlugin*>( plugin );

    if ( iimp->GetSupportedExtensions( iimp, nullptr, &count ) == ErrorTooSmallBuffer )
    {
        extensions = new xstring[count];

        ret = iimp->GetSupportedExtensions( iimp, extensions, &count );
    }

    if ( ret == SuccessCode )
    {
        // copy supported extensions to vector and free xstrings
        mSupportedExtensions.reserve( count );

        for ( int i = 0; i < count; i++ )
        {
            mSupportedExtensions.push_back( string( extensions[i] ) );
            XStringFree( &extensions[i] );
        }
    }

    if ( extensions != 0 )
    {
        // clean up
        delete [] extensions;
    }
}

XImageImportingPlugin::~XImageImportingPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageImportingPlugin> XImageImportingPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageImportingPlugin>( new XImageImportingPlugin( plugin, ownIt ) );
}

// Get some short description of the file type
const string XImageImportingPlugin::GetFileTypeDescription( ) const
{
    string  desc;
    xstring xdesc = nullptr;

    SImageImportingPlugin* iimp = static_cast<SImageImportingPlugin*>( mPlugin );
    xdesc = iimp->GetFileTypeDescription( iimp );

    if ( xdesc != nullptr )
    {
        desc = string( xdesc );
        XStringFree( &xdesc );
    }

    return desc;
}

// Get list of supported file extensions
const vector<string> XImageImportingPlugin::GetSupportedExtensions( ) const
{
    return mSupportedExtensions;
}

// Load image from the specified file
XErrorCode XImageImportingPlugin::ImportImage( const string& fileName, shared_ptr<XImage>& dst ) const
{
    XErrorCode  ret       = ErrorUnsupportedInterface;
    ximage*     dstCImage = ( !dst ) ? nullptr : dst->ImageData( );

    SImageImportingPlugin* iimp = static_cast<SImageImportingPlugin*>( mPlugin );
    ret = iimp->ImportImage( iimp, fileName.c_str( ), &dstCImage );

    if ( !dst )
    {
        // if destination was not allocated before, we do it only on success
        if ( ret == SuccessCode )
        {
            dst = XImage::Create( &dstCImage, true );
        }
        else
        {
            // plug-ins should not really allocate anything new when fail, but lets be safe
            assert( !dstCImage );
            XImageFree( &dstCImage );
        }
    }
    else
    {
        // if destination was already set before, we may need to reset it again
        if ( dst->ImageData( ) != dstCImage )
        {
            // the image we had before was reallocated and so now
            // dst objects wraps a dangling pointer - need to reset it
            dst->Reset( dstCImage );
        }
    }

    return ret;
}
