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

#include <assert.h>
#include "XFamily.hpp"

using namespace std;
using namespace CVSandbox;

XFamily::XFamily( const FamilyDescriptor* desc ) :
    mDescriptor( desc )
{
}

XFamily::~XFamily( )
{
}

const shared_ptr<XFamily> XFamily::Create( const FamilyDescriptor* desc )
{
    assert( desc );
    return shared_ptr<XFamily>( ( desc == 0 ) ? 0 : new XFamily( desc ) );
}

// Get default family of plug-ins'
const shared_ptr<XFamily> XFamily::DefaultFamily( )
{
    shared_ptr<XFamily> family;

    if ( BuiltInPluginFamilitesCount > 0 )
    {
        family = XFamily::Create( BuiltInPluginFamilites[0] );
    }

    return family;
}

const XGuid XFamily::ID( ) const
{
    return XGuid( mDescriptor->ID );
}

const string XFamily::Name( ) const
{
    return std::string( mDescriptor->Name );
}

const string XFamily::Description( ) const
{
    return std::string( mDescriptor->Description );
}

const shared_ptr<const XImage> XFamily::Icon( bool getSmall ) const
{
    return XImage::Create( const_cast<const ximage*>(
        ( getSmall ) ? mDescriptor->SmallIcon : mDescriptor->Icon ) );
}
