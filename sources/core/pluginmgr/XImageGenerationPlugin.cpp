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

#include "XImageGenerationPlugin.hpp"
#include <assert.h>

using namespace std;
using namespace CVSandbox;

XImageGenerationPlugin::XImageGenerationPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageGenerator, ownIt )
{
}

XImageGenerationPlugin::~XImageGenerationPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageGenerationPlugin> XImageGenerationPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageGenerationPlugin>( new XImageGenerationPlugin( plugin, ownIt ) );
}

// Generates an image
XErrorCode XImageGenerationPlugin::GenerateImage( shared_ptr<XImage>& dst )
{
    XErrorCode  ret       = ErrorUnsupportedInterface;
    ximage*     dstCImage = ( !dst ) ? nullptr : dst->ImageData( );

    SImageGenerationPlugin* igp = static_cast<SImageGenerationPlugin*>( mPlugin );
    ret = igp->GenerateImage( igp, &dstCImage );

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
