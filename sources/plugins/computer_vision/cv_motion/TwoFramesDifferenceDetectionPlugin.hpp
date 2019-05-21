/*
    Motion detection plug-ins for Computer Vision Sandbox

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

#pragma once
#ifndef CVS_TWO_FRAMES_DIFFERENCE_DETECTION_PLUGIN_HPP
#define CVS_TWO_FRAMES_DIFFERENCE_DETECTION_PLUGIN_HPP

#include <iplugintypescpp.hpp>

namespace Private
{
    class TwoFramesDifferenceDetectionPluginData;
};

class TwoFramesDifferenceDetectionPlugin : public IDetectionPlugin
{
public:
    TwoFramesDifferenceDetectionPlugin( );
    ~TwoFramesDifferenceDetectionPlugin( );

    // IPluginBase interface
    void Dispose( );

    XErrorCode SetProperty( int32_t id, const xvariant* value );
    XErrorCode GetProperty( int32_t id, xvariant* value ) const;

    // IDetectionPlugin interface

    // Check if the plug-in does changes to input video frames or not
    bool IsReadOnlyMode( );
    // Get pixel formats supported by the plug-in
    XErrorCode GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count );
    // Process the specified image
    XErrorCode ProcessImage( ximage* src );
    // Check if the plug-in triggered detection on the last processed image
    bool Detected( );
    // Reset run time state of the plug-in
    void Reset( );

private:
    static const PropertyDescriptor** propertiesDescription;
    static const XPixelFormat         supportedPixelFormats[];
    Private::TwoFramesDifferenceDetectionPluginData* mData;
};

#endif // CVS_TWO_FRAME_DIFFERENCE_DETECTION_PLUGIN_HPP
