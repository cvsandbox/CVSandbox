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

#include "XGuidGenerator.hpp"
#include <QUuid>

using namespace CVSandbox;

// Generate new unique GUID
XGuid XGuidGenerator::Generate( )
{
    QUuid quuid = QUuid::createUuid( );

    return XGuid(
                quuid.data1,
                ( quuid.data2 << 16 ) + quuid.data3,
                ( quuid.data4[0] << 24 ) + ( quuid.data4[1] << 16 ) + ( quuid.data4[2] << 8 ) + quuid.data4[3],
                ( quuid.data4[4] << 24 ) + ( quuid.data4[5] << 16 ) + ( quuid.data4[6] << 8 ) + quuid.data4[7] );
}
