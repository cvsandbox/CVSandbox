/*
    Image folder plug-ins of Computer Vision Sandbox

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

#include "ImageFolderWriterPlugin.hpp"
#include <chrono>
#include <ctime>
#include <ximaging_formats.h>

using namespace std;
using namespace std::chrono;

// List of supported pixel formats
const XPixelFormat ImageFolderWriterPlugin::supportedPixelFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24
};

namespace Private
{
    // Internals of video file writer class
    class ImageFolderWriterPluginData
    {
    public:
        ImageFolderWriterPluginData( ) :
            FolderToWrite( ), FileNamePrefix( ), BaseFullFileName( ),
            FrameInterval( 1000 ), Codec( 0 ), Quality( 90 ),
            FirstWrite( true ), LastWrite( ),
            LastWriteSeconds( 0 ), CounterValue( 0 )
        {
        }

        void UpdateBaseFullFileName( );

    public:
        string      FolderToWrite;
        string      FileNamePrefix;
        string      BaseFullFileName;

        uint16_t    FrameInterval;
        uint8_t     Codec;
        uint8_t     Quality;

        bool                        FirstWrite;
        steady_clock::time_point    LastWrite;

        uint32_t    LastWriteSeconds;
        uint32_t    CounterValue;

        //steady_clock::time_point                    VideoFileStartTime;
        //steady_clock::time_point                    LastCleanupTime;
    };
}

ImageFolderWriterPlugin::ImageFolderWriterPlugin( ) :
    mData( new ::Private::ImageFolderWriterPluginData( ) )
{
}

ImageFolderWriterPlugin::~ImageFolderWriterPlugin( )
{
    delete mData;
}

void ImageFolderWriterPlugin::Dispose( )
{
    Reset( );

    delete this;
}

// Get specified property value of the plug-in
XErrorCode ImageFolderWriterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->FolderToWrite.c_str( ) );
        break;

    case 1:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->FileNamePrefix.c_str( ) );
        break;

    case 2:
        value->type        = XVT_U2;
        value->value.usVal = mData->FrameInterval;
        break;

    case 3:
        value->type        = XVT_U1;
        value->value.ubVal = mData->Codec;
        break;

    case 4:
        value->type        = XVT_U1;
        value->value.ubVal = mData->Quality;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ImageFolderWriterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = ErrorFailed;
    xvariant    convertedValue;

    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 5, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->FolderToWrite = convertedValue.value.strVal;
            mData->UpdateBaseFullFileName( );
            break;

        case 1:
            mData->FileNamePrefix = convertedValue.value.strVal;
            mData->UpdateBaseFullFileName( );
            break;

        case 2:
            mData->FrameInterval = convertedValue.value.usVal;
            break;

        case 3:
            mData->Codec = convertedValue.value.ubVal;
            break;

        case 4:
            mData->Quality = convertedValue.value.ubVal;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

// Check if the plug-in does changes to input video frames or not
bool ImageFolderWriterPlugin::IsReadOnlyMode( )
{
    return true;
}

// Get pixel formats supported by the video processing plug-in
XErrorCode ImageFolderWriterPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode ImageFolderWriterPlugin::ProcessImage( ximage* src )
{
    XErrorCode               ret     = SuccessCode;
    steady_clock::time_point justNow = steady_clock::now( );

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) )
    {
        // since we declared our own supported formats, we need to check it and ignore if specific codec supports more
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        if ( ( mData->FirstWrite ) || ( duration_cast<std::chrono::milliseconds>( justNow - mData->LastWrite ).count( ) >= mData->FrameInterval ) )
        {
            mData->FirstWrite = false;
            mData->LastWrite  = justNow;

            // convert to tm strcture, so date/time parts are available
            time_t     time = system_clock::to_time_t( system_clock::now( ) );
            struct tm* tm   = localtime( &time );

            uint32_t   writeSeconds = ( ( tm->tm_hour * 60 ) + tm->tm_min ) * 60 + tm->tm_sec;

            if ( writeSeconds == mData->LastWriteSeconds )
            {
                mData->CounterValue++;
            }
            else
            {
                mData->CounterValue = 0;
                mData->LastWriteSeconds = writeSeconds;
            }

            char   buffer[32] = { 0 };
            string fileName   = mData->BaseFullFileName;

            sprintf( buffer, "%04d-%02d-%02d %02d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                                                              tm->tm_hour, tm->tm_min, tm->tm_sec );
            fileName += buffer;

            if ( mData->CounterValue != 0 )
            {
                sprintf( buffer, " (%d)", mData->CounterValue );
                fileName += buffer;
            }

            switch ( mData->Codec )
            {
            case 1:
                fileName += ".png";
                ret = XEncodePng( fileName.c_str( ), src );
                break;

            default:
                fileName += ".jpg";
                ret = XEncodeJpeg( fileName.c_str( ), src, mData->Quality );
                break;
            }
        }
    }

    return ret;
}

// Reset run time state of the video processing plug-in
void ImageFolderWriterPlugin::Reset( )
{
    mData->FirstWrite = true;
}

namespace Private
{
    // Prepare base full file name, which includes folder name and base file name without extension
    void ImageFolderWriterPluginData::UpdateBaseFullFileName( )
    {
        size_t length = FolderToWrite.length( );

        if ( length == 0 )
        {
            BaseFullFileName = FileNamePrefix;
        }
        else
        {
            BaseFullFileName = FolderToWrite;

            if ( ( BaseFullFileName[length - 1] != '/' ) && ( BaseFullFileName[length - 1] != '\\' ) )
            {
                BaseFullFileName += '/';
            }

            BaseFullFileName += FileNamePrefix;
        }
    }
}
