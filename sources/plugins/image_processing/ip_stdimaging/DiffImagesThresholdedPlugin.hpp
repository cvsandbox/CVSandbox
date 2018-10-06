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
#ifndef CVS_DIFF_IMAGES_THRESHOLDED_PLUGIN_HPP
#define CVS_DIFF_IMAGES_THRESHOLDED_PLUGIN_HPP

#include <iplugintypescpp.hpp>

class DiffImagesThresholdedPlugin : public IImageProcessingFilterPlugin2
{
public:
    DiffImagesThresholdedPlugin( );

    // IPluginBase interface
    virtual void Dispose( );

    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IImageProcessingFilterPlugin interface
    virtual bool CanProcessInPlace( );
    virtual XErrorCode GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count );
    virtual xstring GetSecondImageDescription( );
    virtual XSupportedImageSize GetSecondImageSupportedSize( );
    virtual XPixelFormat GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat );
    virtual XErrorCode ProcessImage( const ximage* src, const ximage* src2, ximage** dst );
    virtual XErrorCode ProcessImageInPlace( ximage* src, const ximage* src2 );

private:
    static const PropertyDescriptor**   propertiesDescription;
    static const XPixelFormat           supportedFormats[];
    int16_t  threshold;
    xargb    hiColor;
    xargb    lowColor;
    uint32_t diffPixels;
};

#endif // CVS_DIFF_IMAGES_THRESHOLDED_PLUGIN_HPP
