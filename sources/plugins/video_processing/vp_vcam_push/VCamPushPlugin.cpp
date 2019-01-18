/*
    Virtual camera pushing plug-in for Computer Vision Sandbox

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

#include "VCamPushPlugin.hpp"
#include <ximaging.h>

#ifdef WIN32
    #include <windows.h>
#endif

using namespace std;
using namespace CVSandbox;

// List of supported pixel formats
const XPixelFormat VCamPushPlugin::supportedPixelFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

namespace Private
{
    static const uint32_t   VCAM_MAGIC              = 0xFEEDBABE;

    static const int32_t   VCAM_BPP                 = 24;
    static const int32_t   VCAM_BYTES_PP            = 3;
    static const int32_t   VCAM_MAX_WIDTH           = 2000;
    static const int32_t   VCAM_MAX_HEIGHT          = 1500;

    static const int32_t   VCAM_WIDTH_OFFSET        = VCAM_MAX_WIDTH * VCAM_MAX_HEIGHT * VCAM_BYTES_PP;
    static const int32_t   VCAM_HEIGHT_OFFSET       = VCAM_WIDTH_OFFSET  + sizeof( uint32_t );
    static const int32_t   VCAM_IMAGE_COUNTER       = VCAM_HEIGHT_OFFSET + sizeof( uint32_t );
    static const int32_t   VCAM_MAGIC_OFFSET        = VCAM_IMAGE_COUNTER + sizeof( uint32_t );
    static const int32_t   VCAM_BUF_SIZE            = VCAM_MAGIC_OFFSET  + sizeof( uint32_t );


    static const LPCWSTR    VCAM_SHARED_MEMORY_NAME = L"CVS.VCAM.MEMORY";  // name of shared memory file
    static const LPCWSTR    VCAM_SHARED_MUTEX_NAME  = L"CVS.VCAM.MUTEX";   // name of shared mutex

    // Internals of the virtual camera push plugin
    class VCamPushPluginData : private Uncopyable
    {
    public:
        VCamPushPluginData( ) :
            mutexHandle( NULL ), mapFileHandle( NULL ), sharedBuffer( nullptr ), imageCounter( 0 ),
            rgbImage( nullptr )
        {
        }

        ~VCamPushPluginData( )
        {
            XImageFree( &rgbImage );
        }

        XErrorCode PushImage( const ximage* src );
        void Reset( );

    private:
        bool Initialize( );
        bool IsInitialized( );

    private:
        HANDLE   mutexHandle;
        HANDLE   mapFileHandle;
        uint8_t* sharedBuffer;
        uint32_t imageCounter;
        ximage*  rgbImage;
    };
}

VCamPushPlugin::VCamPushPlugin( ) :
    mData( new ::Private::VCamPushPluginData( ) )
{
}

VCamPushPlugin::~VCamPushPlugin( )
{
    delete mData;
}

void VCamPushPlugin::Dispose( )
{
    Reset( );
    delete this;
}

// Get specified property value of the plug-in
XErrorCode VCamPushPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = ErrorInvalidProperty;

    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ret;
}

// Set specified property value of the plug-in
XErrorCode VCamPushPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = ErrorInvalidProperty;

    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ret;
}

// Check if the plug-in does changes to input video frames or not
bool VCamPushPlugin::IsReadOnlyMode( )
{
    return true;
}

// Get pixel formats supported by the video processing plug-in
XErrorCode VCamPushPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode VCamPushPlugin::ProcessImage( ximage* src )
{
    return mData->PushImage( src );
}

// Reset run time state of the video processing plug-in
void VCamPushPlugin::Reset( )
{
    mData->Reset( );
}

namespace Private
{

// Initialize shared mutex and memory
bool VCamPushPluginData::Initialize( )
{
    bool ret = false;

    mutexHandle = CreateMutexW( NULL, FALSE, VCAM_SHARED_MUTEX_NAME );

    if ( mutexHandle != NULL )
    {
        if ( WaitForSingleObject( mutexHandle, INFINITE ) == WAIT_OBJECT_0 )
        {
            mapFileHandle = CreateFileMappingW(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                VCAM_BUF_SIZE,           // maximum object size (low-order DWORD)
                VCAM_SHARED_MEMORY_NAME );

            if ( mapFileHandle != NULL )
            {
                sharedBuffer = static_cast<uint8_t*>( MapViewOfFile( mapFileHandle, FILE_MAP_ALL_ACCESS, 0, 0, VCAM_BUF_SIZE ) );

                if ( sharedBuffer != nullptr )
                {
                    sharedBuffer[VCAM_BUF_SIZE - 1] = 0;
                    ret = true;
                }
            }

            ReleaseMutex( mutexHandle );
        }
    }

    if ( !ret )
    {
        Reset( );
    }

    return ret;
}

// Free both shared mutex handle and memory handle
void VCamPushPluginData::Reset( )
{
    if ( mutexHandle != NULL )
    {
        if ( WaitForSingleObject( mutexHandle, INFINITE ) == WAIT_OBJECT_0 )
        {
            if ( sharedBuffer != nullptr )
            {
                sharedBuffer[VCAM_BUF_SIZE - 1] = 0;
                UnmapViewOfFile( sharedBuffer );
                sharedBuffer = nullptr;
            }
            if ( mapFileHandle != NULL )
            {
                CloseHandle( mapFileHandle );
                mapFileHandle = NULL;
            }

            ReleaseMutex( mutexHandle );
        }

        CloseHandle( mutexHandle );
        mutexHandle = NULL;
    }
}

// Check if initialization was done successfully
bool VCamPushPluginData::IsInitialized( )
{
    return ( sharedBuffer != nullptr );
}

// Push provided image into virtual camera
XErrorCode VCamPushPluginData::PushImage( const ximage* src )
{
    XErrorCode      ret         = ErrorInitializationFailed;
    const ximage*   imageToPush = src;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width > VCAM_MAX_WIDTH ) || ( src->height > VCAM_MAX_HEIGHT ) )
    {
        ret = ErrorImageIsTooBig;
    }
    else
    {
        if ( !IsInitialized( ) )
        {
            Initialize( );
        }

        if ( IsInitialized( ) )
        {
            // convert grayscale to RGB if required
            if ( imageToPush->format == XPixelFormatGrayscale8 )
            {
                // free old temporary image, if its size is not the one we need
                if ( ( rgbImage != nullptr ) && ( ( rgbImage->width != imageToPush->width ) || ( rgbImage->height != imageToPush->height ) ) )
                {
                    XImageFree( &rgbImage );
                }

                if ( rgbImage == nullptr )
                {
                    ret = XImageAllocateRaw( imageToPush->width, imageToPush->height, XPixelFormatRGB24, &rgbImage );
                }

                if ( rgbImage != nullptr )
                {
                    GrayscaleToColor( imageToPush, rgbImage );
                    imageToPush = rgbImage;
                }
                else
                {
                    ret = ErrorOutOfMemory;
                    imageToPush = nullptr;
                }
            }

            // push an image if there is any
            if ( imageToPush != nullptr )
            {
                if ( WaitForSingleObject( mutexHandle, INFINITE ) == WAIT_OBJECT_0 )
                {
                    int width     = imageToPush->width;
                    int height    = imageToPush->height;
                    int dstStride = width * VCAM_BYTES_PP;
                    int srcStride = imageToPush->stride;

                    if ( imageToPush->format == XPixelFormatRGB24 )
                    {
                        // push 24 bpp image
                        for ( int y = 0; y < height; y++ )
                        {
                            uint8_t* dstRow = sharedBuffer      + y * dstStride;
                            uint8_t* srcRow = imageToPush->data + y * srcStride;

                            memcpy( dstRow, srcRow, dstStride );
                        }
                    }
                    else
                    {
                        // push 32 bpp image
                        for ( int y = 0; y < height; y++ )
                        {
                            uint8_t* dstRow = sharedBuffer      + y * dstStride;
                            uint8_t* srcRow = imageToPush->data + y * srcStride;

                            for ( int x = 0; x < width; x++, dstRow += 3, srcRow += 4 )
                            {
                                float alpha = srcRow[AlphaIndex] / 255.0f;

                                dstRow[RedIndex]   = static_cast<uint8_t>( alpha * srcRow[RedIndex] );
                                dstRow[GreenIndex] = static_cast<uint8_t>( alpha * srcRow[GreenIndex] );
                                dstRow[BlueIndex]  = static_cast<uint8_t>( alpha * srcRow[BlueIndex] );
                            }
                        }
                    }

                    if ( ++imageCounter == 0 ) imageCounter = 1;

                    *( (uint32_t*)( sharedBuffer + VCAM_WIDTH_OFFSET  ) ) = static_cast<uint32_t>( width );
                    *( (uint32_t*)( sharedBuffer + VCAM_HEIGHT_OFFSET ) ) = static_cast<uint32_t>( height );
                    *( (uint32_t*)( sharedBuffer + VCAM_IMAGE_COUNTER ) ) = imageCounter;
                    *( (uint32_t*)( sharedBuffer + VCAM_MAGIC_OFFSET  ) ) = VCAM_MAGIC;

                    ret = SuccessCode;

                    ReleaseMutex( mutexHandle );
                }
            }
        }
    }

    return ret;
}

}
