/*
    Automation server library of Computer Vision Sandbox

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
#ifndef CVS_XVIDEO_SOURCE_FRAME_INFO_HPP
#define CVS_XVIDEO_SOURCE_FRAME_INFO_HPP

#include <stdint.h>
#include <ximage.h>

namespace CVSandbox { namespace Automation
{

struct XVideoSourceFrameInfo
{
    // Total number of video frames received
    uint32_t     FramesReceived;
    uint32_t     FramesDropped;
    uint32_t     FramesBlocked;
    int32_t      OriginalFrameWidth;
    int32_t      OriginalFrameHeight;
    XPixelFormat OriginalPixelFormat;
    int32_t      ProcessedFrameWidth;
    int32_t      ProcessedFrameHeight;
    XPixelFormat ProcessedPixelFormat;
    uint32_t     VideoProcessingStepsDone;

    XVideoSourceFrameInfo( ) :
        FramesReceived( 0 ), FramesDropped( 0 ), FramesBlocked( 0 ),
        OriginalFrameWidth( 0 ), OriginalFrameHeight( 0 ), OriginalPixelFormat( XPixelFormatUnknown ),
        ProcessedFrameWidth( 0 ), ProcessedFrameHeight( 0 ), ProcessedPixelFormat( XPixelFormatUnknown ),
        VideoProcessingStepsDone( 0 )
    {
    }
};

} } // namespace CVSandbox::Automation

#endif // CVS_XVIDEO_SOURCE_FRAME_INFO_HPP