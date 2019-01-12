/*
    Screen capture plug-ins of Computer Vision Sandbox

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

#include "ScreenCapturePlugin.hpp"
#include <memory.h>
#include <string>
#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <Windows.h>
#include <ximaging.h>
#include <chrono>
#include <memory>

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Threading;

namespace Private
{
    static const char* STR_ERROR_OUT_OF_MEMORY = "Out of memory";
    static const char* STR_ERROR_INIT_FAILED   = "Initialization failed";

    // Internal class which hides private parts of the ScreenCapturePlugin class,
    // so those are not exposed in the main class
    class ScreenCapturePluginData
    {
    public:
        ScreenCapturePluginData( ) : UserCallbacks( { 0 } ), UserParam( nullptr ),
            CaptureMode( 0 ), ScreenToCapture( 0 ),
            CapturePoint( { 0, 0 } ), CaptureSize( { 640, 480 } ), FrameRate( 20 ),
            WindowTitle( ), WindowTitleUtf16( ), WindowHandle( NULL ),
            ResizeImage( false ), KeepAspectRatio( true ), OutputSize( { 640, 480 } ),
            Displays( ScreenCapturePlugin::GetDisplays( ) )

        {
            if ( !Displays.empty( ) )
            {
                xrect firstDisplay = Displays.front( );

                CapturePoint.x = firstDisplay.x1;
                CapturePoint.y = firstDisplay.y1;
                CaptureSize.width  = firstDisplay.x2 - firstDisplay.x1;
                CaptureSize.height = firstDisplay.y2 - firstDisplay.y1;
            }
        }

        ~ScreenCapturePluginData( )
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

        uint8_t             CaptureMode;
        uint8_t             ScreenToCapture;
        xpoint              CapturePoint;
        xsize               CaptureSize;
        uint16_t            FrameRate;
        string              WindowTitle;
        unique_ptr<WCHAR>   WindowTitleUtf16;
        HWND                WindowHandle;
        bool                ResizeImage;
        bool                KeepAspectRatio;
        xsize               OutputSize;
        list<xrect>         Displays;

        XMutex              Sync;
        XManualResetEvent   ExitEvent;
        XThread             BackgroundThread;
        uint32_t            FramesCounter;
    };
}

// ==========================================================================

ScreenCapturePlugin::ScreenCapturePlugin( ) :
    mData( new ::Private::ScreenCapturePluginData( ) )
{
}

ScreenCapturePlugin::~ScreenCapturePlugin( )
{
    delete mData;
}

void ScreenCapturePlugin::Dispose( )
{
    delete this;
}

// Start video source so it initializes and begins providing video frames
XErrorCode ScreenCapturePlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    mData->FramesCounter = 0;
    mData->ExitEvent.Reset( );

    if ( mData->BackgroundThread.Create( ::Private::ScreenCapturePluginData::WorkerThreadHandler, mData ) )
    {
        ret = SuccessCode;
    }

    return ret;
}

// Signal video to stop, so it could finalize and clean-up
void ScreenCapturePlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void ScreenCapturePlugin::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its thread) is still running
bool ScreenCapturePlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void ScreenCapturePlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t ScreenCapturePlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set callbacks for the video source
void ScreenCapturePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
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
XErrorCode ScreenCapturePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = mData->CaptureMode;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = mData->ScreenToCapture;
        break;

    case 2:
        value->type = XVT_Point;
        value->value.pointVal = mData->CapturePoint;
        break;

    case 3:
        value->type = XVT_Size;
        value->value.sizeVal = mData->CaptureSize;
        break;

    case 4:
        value->type = XVT_String;
        value->value.strVal = XStringAlloc( mData->WindowTitle.c_str( ) );
        break;

    case 5:
        value->type = XVT_U2;
        value->value.usVal = mData->FrameRate;
        break;

    case 6:
        value->type = XVT_Bool;
        value->value.boolVal = mData->ResizeImage;
        break;

    case 7:
        value->type = XVT_Size;
        value->value.sizeVal = mData->OutputSize;
        break;

    case 8:
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
XErrorCode ScreenCapturePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = SuccessCode;
    XScopedLock lock( &mData->Sync );

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 9, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->CaptureMode = XINRANGE( convertedValue.value.ubVal, 0, 2 );
            break;

        case 1:
            if ( !mData->Displays.empty( ) )
            {
                mData->ScreenToCapture = (uint8_t) XINRANGE( convertedValue.value.ubVal, 0, mData->Displays.size( ) - 1 );
            }
            break;

        case 2:
            mData->CapturePoint = convertedValue.value.pointVal;
            break;

        case 3:
            mData->CaptureSize = convertedValue.value.sizeVal;
            break;

        case 4:
            mData->WindowTitle = convertedValue.value.strVal;
            break;

        case 5:
            mData->FrameRate = XINRANGE( convertedValue.value.usVal, 1, 60 );
            break;

        case 6:
            mData->ResizeImage = convertedValue.value.boolVal;
            break;

        case 7:
            mData->OutputSize = convertedValue.value.sizeVal;
            break;

        case 8:
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

// Callback to enumerate displays
BOOL CALLBACK MonitorEnumProc( HMONITOR, HDC, LPRECT lprcMonitor, LPARAM dwData )
{
    list<xrect>* displays = reinterpret_cast<list<xrect>*>( dwData );

    displays->push_back( { lprcMonitor->left, lprcMonitor->top, lprcMonitor->right, lprcMonitor->bottom } );

    return TRUE;
}

// Enumerate available displays
list<xrect> ScreenCapturePlugin::GetDisplays( )
{
    list<xrect> displays;

    EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>( &displays ) );

    return displays;
}

namespace Private
{
    // Video thread entry point
    void ScreenCapturePluginData::WorkerThreadHandler( void* param )
    {
        static_cast<ScreenCapturePluginData*>( param )->VideoSourceWorker( );
    }

    // Notify client about new video frame
    void ScreenCapturePluginData::NewFrameNotify( const ximage* image )
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
    void ScreenCapturePluginData::ErrorMessageNotify( const char* errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( UserCallbacks.ErrorMessageCallback != nullptr )
        {
            UserCallbacks.ErrorMessageCallback( UserParam, errorMessage );
        }
    }

    // Callback for enumerating windows
    static BOOL CALLBACK FindWindow( HWND hwnd, LPARAM lParam )
    {
        ScreenCapturePluginData* self = reinterpret_cast<ScreenCapturePluginData*>( lParam );

        if ( self->WindowHandle == NULL )
        {
            WCHAR title[256] = { 0 };

            GetWindowTextW( hwnd, title, sizeof( title ) );
            CharUpperW( title );

            if ( wcsstr( title, self->WindowTitleUtf16.get( ) ) )
            {
                self->WindowHandle = hwnd;
            }
        }

        return TRUE;
    }

    // Get rectangle of the specified window
    bool ScreenCapturePluginData::GetWindowRectangle( xrect* windowRect )
    {
        bool ret = false;
        RECT wr  = { 0 };

        if ( ( WindowHandle == NULL ) || ( GetWindowRect( WindowHandle, &wr ) != TRUE ) )
        {
            WindowHandle = NULL;

            EnumWindows( FindWindow, reinterpret_cast<LPARAM>( this ) );

            if ( ( WindowHandle != NULL ) && ( GetWindowRect( WindowHandle, &wr ) != TRUE ) )
            {
                WindowHandle = NULL;
            }
        }

        if ( WindowHandle != NULL )
        {
            windowRect->x1 = wr.left;
            windowRect->y1 = wr.top;
            windowRect->x2 = wr.right;
            windowRect->y2 = wr.bottom;

            ret = true;
        }

        return ret;
    }

    // Run video loop in a background thread
    void ScreenCapturePluginData::VideoSourceWorker( )
    {
        ximage* capturedImage   = nullptr;
        ximage* resizedImage    = nullptr;

        uint8_t captureMode     = 0;
        uint8_t screenToCapture = 0;
        int32_t screenx, screeny, width, height, outputWidth, outputHeight;
        bool    resizeImage, keepAspectRatio;

        // get copies of the properties we need
        {
            XScopedLock lock( &Sync );

            captureMode = CaptureMode;
            screenToCapture = ScreenToCapture;

            screenx = CapturePoint.x;
            screeny = CapturePoint.y;
            width   = CaptureSize.width;
            height  = CaptureSize.height;

            resizeImage      = ResizeImage;
            keepAspectRatio  = KeepAspectRatio;
            outputWidth      = OutputSize.width;
            outputHeight     = OutputSize.height;

            // convert to wide characters
            int charsRequired = MultiByteToWideChar( CP_UTF8, 0, WindowTitle.c_str( ), -1, NULL, 0 );

            WindowTitleUtf16 = unique_ptr<WCHAR>( (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired ) );

            MultiByteToWideChar( CP_UTF8, 0, WindowTitle.c_str( ), -1, WindowTitleUtf16.get( ), charsRequired );
            CharUpperW( WindowTitleUtf16.get( ) );
        }

        if ( ( captureMode == 0 ) && ( Displays.size( ) > screenToCapture ) )
        {
            list<xrect>::iterator it = Displays.begin( );
            std::advance( it, screenToCapture );

            xrect display = *it;

            screenx = display.x1;
            screeny = display.y1;
            width   = display.x2 - display.x1;
            height  = display.y2 - display.y1;
        }

        uint32_t timeBetweenFrames = 1000 / FrameRate;
        uint32_t timeToSleep = 0;
        uint32_t timeTaken;

        if ( ( XImageAllocateRaw( width, height, XPixelFormatRGB24, &capturedImage ) != SuccessCode ) ||
             ( ( resizeImage ) && ( XImageAllocateRaw( outputWidth, outputHeight, XPixelFormatRGB24, &resizedImage ) != SuccessCode ) ) )
        {
            ErrorMessageNotify( STR_ERROR_OUT_OF_MEMORY );
        }
        else
        {
            // get the desktop device context
            HDC hdc = GetDC( NULL );
            // create a device context to use yourself
            HDC hDest = CreateCompatibleDC( hdc );
            // create a bitmap
            HBITMAP hDesktop = CreateCompatibleBitmap( hdc, width, height );

            if ( ( hdc == NULL ) || ( hDest == NULL ) || ( hDesktop == NULL ) )
            {
                ErrorMessageNotify( STR_ERROR_INIT_FAILED );
            }
            else
            {
                // use the previously created device context with the bitmap
                HGDIOBJ hOldObject = SelectObject( hDest, hDesktop );

                BITMAPINFOHEADER bmi = { 0 };

                bmi.biSize        = sizeof( BITMAPINFOHEADER );
                bmi.biPlanes      = 1;
                bmi.biBitCount    = 24;
                bmi.biCompression = BI_RGB;

                do
                {
                    steady_clock::time_point captureStartTime = steady_clock::now( );

                    // if we capture window, it is required to track its size/position
                    if ( captureMode == 2 )
                    {
                        xrect windowRect;

                        if ( GetWindowRectangle( &windowRect ) )
                        {
                            screenx = windowRect.x1;
                            screeny = windowRect.y1;
                            width   = windowRect.x2 - windowRect.x1;
                            height  = windowRect.y2 - windowRect.y1;


                            if ( ( width == 0 ) || ( height == 0 ) )
                            {
                                WindowHandle = NULL;
                            }
                            else
                            {
                                if ( ( width != capturedImage->width ) || ( height != capturedImage->height ) )
                                {
                                    if ( XImageAllocateRaw( width, height, XPixelFormatRGB24, &capturedImage ) != SuccessCode )
                                    {
                                        ErrorMessageNotify( STR_ERROR_OUT_OF_MEMORY );
                                    }

                                    // create new bitmap
                                    SelectObject( hDest, hOldObject );
                                    DeleteObject( hDesktop );
                                    hDesktop = CreateCompatibleBitmap( hdc, width, height );
                                    hOldObject = SelectObject( hDest, hDesktop );

                                    if ( hDesktop == NULL )
                                    {
                                        ErrorMessageNotify( STR_ERROR_INIT_FAILED );
                                    }
                                }
                            }
                        }
                    }

                    // make sure we still have the image to capture to
                    if ( ( capturedImage != nullptr ) && ( hDesktop != NULL ) )
                    {
                        if ( ( captureMode == 2 ) && ( WindowHandle == NULL ) )
                        {
                            XDrawingFillImage( ( resizeImage ) ? resizedImage : capturedImage, { 0xFF000000 } );
                            XDrawingText( ( resizeImage ) ? resizedImage : capturedImage, "Window was not found", 0, 0, { 0xFFFFFFFF }, { 0xFF000000 }, true );
                        }
                        else
                        {
                            // copy from the desktop device context to the bitmap device context
                            BitBlt( hDest, 0, 0, width, height, hdc, screenx, screeny, SRCCOPY );

                            // complete bitmap header
                            bmi.biWidth     = width;
                            bmi.biHeight    = -height;
                            bmi.biSizeImage = width * height;

                            // get bitmap data
                            GetDIBits( hdc, hDesktop, 0, height, capturedImage->data, (BITMAPINFO*) &bmi, DIB_RGB_COLORS );
                            SwapRedBlue( capturedImage );

                            if ( resizeImage )
                            {
                                float xRatio = static_cast<float>( width )  / outputWidth;
                                float yRatio = static_cast<float>( height ) / outputHeight;
                                float ratio  = ( xRatio > yRatio ) ? xRatio : yRatio;
                                int   outW   = outputWidth;
                                int   outH   = outputHeight;

                                if ( xRatio > yRatio )
                                {
                                    outH = static_cast<int>( height / ratio );
                                }
                                else
                                {
                                    outW = static_cast<int>( width / ratio );
                                }

                                if ( ( ( outW == outputWidth ) && ( outH == outputHeight ) ) ||
                                     ( !keepAspectRatio ) )
                                {
                                    ResizeImageBilinear( capturedImage, resizedImage );
                                }
                                else
                                {
                                    ximage* subImage = nullptr;

                                    if ( XImageGetSubImage( resizedImage, &subImage, 0, 0, outW, outH ) != SuccessCode )
                                    {
                                        ErrorMessageNotify( STR_ERROR_OUT_OF_MEMORY );
                                    }
                                    else
                                    {
                                        ResizeImageBilinear( capturedImage, subImage );
                                        XImageFree( &subImage );

                                        if ( outW == outputWidth )
                                        {
                                            XDrawingFillRectanle( resizedImage, 0, outH, outputWidth - 1, outputHeight - 1, { 0xFF000000 } );
                                        }
                                        else
                                        {
                                            XDrawingFillRectanle( resizedImage, outW, 0, outputWidth - 1, outputHeight - 1, { 0xFF000000 } );
                                        }
                                    }
                                }
                            }
                        }

                        NewFrameNotify( ( resizeImage ) ? resizedImage : capturedImage );
                    }

                    // decide how much to sleep
                    timeTaken   = static_cast<uint32_t>( duration_cast<std::chrono::milliseconds>( steady_clock::now( ) - captureStartTime ).count( ) );
                    timeToSleep = ( timeTaken > timeBetweenFrames ) ? 0 : timeBetweenFrames - timeTaken;
                }
                while ( !ExitEvent.Wait( timeToSleep ) );
            }

            // release resources
            DeleteObject( hDesktop );
            DeleteDC( hDest );
            ReleaseDC( NULL, hdc );
        }

        XImageFree( &capturedImage );
        XImageFree( &resizedImage );
    }
}
