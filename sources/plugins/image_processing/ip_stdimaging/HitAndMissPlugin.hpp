/*
    Standard image processing plug-ins of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_HIT_AND_MISS_PLUGIN_HPP
#define CVS_HIT_AND_MISS_PLUGIN_HPP

#include <ximaging.h>
#include <iplugintypescpp.hpp>

class HitAndMissPlugin : public IImageProcessingFilterPlugin
{
private:
    ~HitAndMissPlugin( );

    enum class OperatorType
    {
        HitAndMiss = 0,
        Thinning   = 1,
        Thickening = 2,
    };

public:

    HitAndMissPlugin( );

    // IPluginBase interface
    void Dispose( );

    XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IImageProcessingFilterPlugin interface
    bool CanProcessInPlace( );
    XErrorCode GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count );
    XErrorCode ProcessImage( const ximage* src, ximage** dst );
    XErrorCode ProcessImageInPlace( ximage* src );

private:
    static const PropertyDescriptor** propertiesDescription;
    static const XPixelFormat         supportedFormats[];
    uint32_t                          structuringElementSize;
    int8_t*                           structuringElement;
    OperatorType                      operatorType;
};

#endif // CVS_MORPHOLOGY_OPERATOR_PLUGIN_HPP
