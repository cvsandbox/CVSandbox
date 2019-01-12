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

#include "ImageFolderVideoSourcePlugin.hpp"
#include <memory.h>
#include <string>
#include <chrono>
#include <memory>
#include <list>
#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XError.hpp>
#include <ximaging.h>
#include <ximaging_formats.h>
#include <Windows.h>

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Threading;

namespace Private
{
    static const char* STR_ERROR_OUT_OF_MEMORY = "Out of memory";

    // Internal class which hides private parts of the ScreenCapturePlugin class,
    // so those are not exposed in the main class
    class ImageFolderVideoSourcePluginData
    {
    public:
        ImageFolderVideoSourcePluginData( ) : UserCallbacks( { 0 } ), UserParam( nullptr ),
            ImageType( 0 ), ImageFolder( ), FrameInterval( 1000 ), CycleImages( false ),
            ResizeImage( false ), KeepAspectRatio( true ), OutputSize( { 640, 480 } )

        {

        }

        ~ImageFolderVideoSourcePluginData( )
        {
        }

        // Video thread entry point
        static void WorkerThreadHandler( void* param );
        // Notify client about new video frame
        void NewFrameNotify( const ximage* image );
        // Notify client about error in the video source
        void ErrorMessageNotify( const char* errorMessage );
        // Get rectangle of the specified window
        bool GetWindowRectangle( xrect* windowRect );
        // Run video loop in a background worker thread
        void VideoSourceWorker( );

    public:
        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;

        uint8_t             ImageType;
        string              ImageFolder;
        uint16_t            FrameInterval;
        bool                CycleImages;
        bool                ResizeImage;
        bool                KeepAspectRatio;
        xsize               OutputSize;

        XMutex              Sync;
        XManualResetEvent   ExitEvent;
        XThread             BackgroundThread;
        uint32_t            FramesCounter;
    };
}

// ==========================================================================

ImageFolderVideoSourcePlugin::ImageFolderVideoSourcePlugin( ) :
    mData( new ::Private::ImageFolderVideoSourcePluginData( ) )
{
}

ImageFolderVideoSourcePlugin::~ImageFolderVideoSourcePlugin( )
{
    delete mData;
}

void ImageFolderVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Start video source so it initializes and begins providing video frames
XErrorCode ImageFolderVideoSourcePlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    mData->FramesCounter = 0;
    mData->ExitEvent.Reset( );

    if ( mData->BackgroundThread.Create( ::Private::ImageFolderVideoSourcePluginData::WorkerThreadHandler, mData ) )
    {
        ret = SuccessCode;
    }

    return ret;
}

// Signal video to stop, so it could finalize and clean-up
void ImageFolderVideoSourcePlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void ImageFolderVideoSourcePlugin::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its thread) is still running
bool ImageFolderVideoSourcePlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void ImageFolderVideoSourcePlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t ImageFolderVideoSourcePlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set callbacks for the video source
void ImageFolderVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
{
    XScopedLock lock( &mData->Sync );

    if ( callbacks != 0 )
    {
        memcpy( &mData->UserCallbacks, callbacks, sizeof( mData->UserCallbacks ) );
        mData->UserParam = userParam;
    }
    else
    {
        memset( &mData->UserCallbacks, 0, sizeof( mData->UserCallbacks ) );
        mData->UserParam = 0;
    }
}

// Get specified property value of the plug-in
XErrorCode ImageFolderVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = mData->ImageType;
        break;

    case 1:
        value->type = XVT_String;
        value->value.strVal = XStringAlloc( mData->ImageFolder.c_str( ) );
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = mData->FrameInterval;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = mData->CycleImages;
        break;

    case 4:
        value->type = XVT_Bool;
        value->value.boolVal = mData->ResizeImage;
        break;

    case 5:
        value->type = XVT_Size;
        value->value.sizeVal = mData->OutputSize;
        break;

    case 6:
        value->type = XVT_Bool;
        value->value.boolVal = mData->KeepAspectRatio;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ImageFolderVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = SuccessCode;
    XScopedLock lock( &mData->Sync );

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 7, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->ImageType = convertedValue.value.ubVal;
            break;

        case 1:
            mData->ImageFolder = string( convertedValue.value.strVal );
            break;

        case 2:
            mData->FrameInterval = convertedValue.value.usVal;
            break;

        case 3:
            mData->CycleImages = convertedValue.value.boolVal;
            break;

        case 4:
            mData->ResizeImage = convertedValue.value.boolVal;
            break;

        case 5:
            mData->OutputSize = convertedValue.value.sizeVal;
            break;

        case 6:
            mData->KeepAspectRatio = convertedValue.value.boolVal;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

namespace Private
{

// Video thread entry point
void ImageFolderVideoSourcePluginData::WorkerThreadHandler( void* param )
{
    static_cast<ImageFolderVideoSourcePluginData*>( param )->VideoSourceWorker( );
}

// Notify client about new video frame
void ImageFolderVideoSourcePluginData::NewFrameNotify( const ximage* image )
{
    XScopedLock lock( &Sync );

    FramesCounter++;

    // provide image only if someone needs it
    if ( UserCallbacks.NewImageCallback != nullptr )
    {
        UserCallbacks.NewImageCallback( UserParam, image );
    }
}

// Notify client about error in the video source
void ImageFolderVideoSourcePluginData::ErrorMessageNotify( const char* errorMessage )
{
    XScopedLock lock( &Sync );

    if ( UserCallbacks.ErrorMessageCallback != nullptr )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, errorMessage );
    }
}

// Convert specfied wide character string to UTF8
const string Utf16to8( wstring utf16string )
{
    string ret;

    int bytesRequired = WideCharToMultiByte( CP_UTF8, 0, utf16string.c_str( ), -1, NULL, 0, NULL, NULL );

    if ( bytesRequired > 0 )
    {
        char* utf8string = new char[bytesRequired];

        if ( WideCharToMultiByte( CP_UTF8, 0, utf16string.c_str( ), -1, utf8string, bytesRequired, NULL, NULL ) > 0 )
        {
            ret = string( utf8string );
        }

        delete[] utf8string;
    }

    return ret;
}

// Collect JPEG file names to browse (provide them in UTF8, which is what JPEG decoder in used expects)
static list<string> CollectImageFiles( const wstring searchFolder, uint8_t imageType )
{
    wstring      folder = searchFolder;
    wstring      searchPattern;
    list<string> imageFiles;

    if ( ( folder.length( ) != 0 ) && ( folder.at( folder.length( ) - 1 ) != L'\\' ) )
    {
        folder += L'\\';
    }
    searchPattern = folder + L'*';

    // find all files satisfying the search string
    HANDLE           hFind;
    WIN32_FIND_DATAW ffData;
    wchar_t          ext[5] = { 0 };

    if ( ( hFind = FindFirstFileW( searchPattern.c_str( ), &ffData ) ) != INVALID_HANDLE_VALUE )
    {
        do
        {
            if ( ( ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            {
                wchar_t* extStart = wcsrchr( ffData.cFileName, L'.' );

                if ( ( extStart != nullptr ) && ( extStart[1] != L'\0' ) && ( wcslen( extStart ) <= 5 ) )
                {
                    wcscpy( ext, &extStart[1] );
                    CharUpperW( ext );

                    if ( ( ( imageType == 0 ) && ( ( wcscmp( ext, L"JPG" ) == 0 ) || ( wcscmp( ext, L"JPEG" ) == 0 ) ) ) ||
                         ( ( imageType == 1 ) && ( wcscmp( ext, L"PNG" ) == 0 ) ) )
                    {
                        imageFiles.push_back( Utf16to8( folder + ffData.cFileName ) );
                    }
                }
            }
        }
        while ( FindNextFileW( hFind, &ffData ) );

        FindClose( hFind );
    }

    imageFiles.sort( );

    return imageFiles;
}

// Run video loop in a background thread
void ImageFolderVideoSourcePluginData::VideoSourceWorker( )
{
    ximage* loadedImage     = nullptr;
    ximage* resizedImage    = nullptr;

    uint8_t  imageType;
    uint16_t frameInterval;
    int32_t  outputWidth, outputHeight;
    bool     resizeImage, keepAspectRatio, cycleImages;
    wstring  searchFolder;

    // get copies of the properties we need
    {
        XScopedLock lock( &Sync );

        imageType       = ImageType;
        frameInterval   = FrameInterval;
        cycleImages     = CycleImages;
        resizeImage     = ResizeImage;
        keepAspectRatio = KeepAspectRatio;
        outputWidth     = OutputSize.width;
        outputHeight    = OutputSize.height;

        // convert to wide characters
        int charsRequired = MultiByteToWideChar( CP_UTF8, 0, ImageFolder.c_str( ), -1, NULL, 0 );
        unique_ptr<WCHAR> folderUtf16 = unique_ptr<WCHAR>( (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired ) );

        MultiByteToWideChar( CP_UTF8, 0, ImageFolder.c_str( ), -1, folderUtf16.get( ), charsRequired );
        searchFolder = wstring( folderUtf16.get( ) );
    }

    if ( ( resizeImage ) && ( XImageAllocateRaw( outputWidth, outputHeight, XPixelFormatRGB24, &resizedImage ) != SuccessCode ) )
    {
        ErrorMessageNotify( STR_ERROR_OUT_OF_MEMORY );
    }
    else
    {
        list<string> imageFiles = CollectImageFiles( searchFolder, imageType );
        list<string>::const_iterator itFiles = imageFiles.begin( );

        if ( itFiles == imageFiles.end( ) )
        {
            ErrorMessageNotify( "No image files found" );
        }
        else
        {
            uint32_t timeBetweenFrames = frameInterval;
            uint32_t timeToSleep       = 0;
            uint32_t timeTaken;

            do
            {
                steady_clock::time_point captureStartTime = steady_clock::now( );

                if ( itFiles == imageFiles.end( ) )
                {
                    if ( cycleImages )
                    {
                        itFiles = imageFiles.begin( );
                    }
                    else
                    {
                        ErrorMessageNotify( "No more images left" );
                        break;
                    }
                }

                // load next image
                XErrorCode ecode = ( imageType == 0 ) ? XDecodeJpeg( itFiles->c_str( ), &loadedImage ) :
                                   ( imageType == 1 ) ? XDecodePng( itFiles->c_str( ), &loadedImage ) :
                                   ErrorInvalidConfiguration;

                if ( ecode != SuccessCode )
                {
                    ErrorMessageNotify( XError::Description( ecode ).c_str( ) );
                }
                else
                {
                    if ( resizeImage )
                    {
                        int32_t width  = loadedImage->width;
                        int32_t height = loadedImage->height;
                        float   xRatio = static_cast<float>( width )  / outputWidth;
                        float   yRatio = static_cast<float>( height ) / outputHeight;
                        float   ratio  = ( xRatio > yRatio ) ? xRatio : yRatio;
                        int     outW   = outputWidth;
                        int     outH   = outputHeight;

                        if ( xRatio > yRatio )
                        {
                            outH = static_cast<int>( height / ratio );
                        }
                        else
                        {
                            outW = static_cast<int>( width / ratio );
                        }

                        ximage* tempImage = resizedImage;

                        // Most of the time 24 bpp JPEGs are expected. However we may need to be prepared for grayscale as well.
                        if ( tempImage->format != loadedImage->format )
                        {
                            tempImage = nullptr;
                            ecode = XImageAllocateRaw( outputWidth, outputHeight, loadedImage->format, &tempImage );
                        }

                        if ( ecode == SuccessCode )
                        {
                            if ( ( ( outW == outputWidth ) && ( outH == outputHeight ) ) ||
                                 ( !keepAspectRatio ) )
                            {
                                ecode = ResizeImageBilinear( loadedImage, tempImage );
                            }
                            else
                            {
                                ximage* subImage = nullptr;

                                ecode = XImageGetSubImage( tempImage, &subImage, 0, 0, outW, outH );

                                if ( ecode == SuccessCode )
                                {
                                    ecode = ResizeImageBilinear( loadedImage, subImage );
                                    XImageFree( &subImage );

                                    if ( ecode == SuccessCode )
                                    {
                                        if ( outW == outputWidth )
                                        {
                                            XDrawingFillRectanle( tempImage, 0, outH, outputWidth - 1, outputHeight - 1, { 0xFF000000 } );
                                        }
                                        else
                                        {
                                            XDrawingFillRectanle( tempImage, outW, 0, outputWidth - 1, outputHeight - 1, { 0xFF000000 } );
                                        }
                                    }
                                }
                            }
                        }

                        if ( ecode != SuccessCode )
                        {
                            ErrorMessageNotify( XError::Description( ecode ).c_str( ) );
                        }
                        else
                        {
                            NewFrameNotify( tempImage );
                        }

                        // free temporary image if required
                        if ( tempImage != resizedImage )
                        {
                            XImageFree( &tempImage );
                        }
                    }
                    else
                    {
                        NewFrameNotify( loadedImage );
                    }
                }

                ++itFiles;

                // decide how much to sleep
                timeTaken   = static_cast<uint32_t>( duration_cast<std::chrono::milliseconds>( steady_clock::now( ) - captureStartTime ).count( ) );
                timeToSleep = ( timeTaken > timeBetweenFrames ) ? 0 : timeBetweenFrames - timeTaken;
            }
            while ( !ExitEvent.Wait( timeToSleep ) );
        }
    }

    XImageFree( &loadedImage );
    XImageFree( &resizedImage );
}

}
