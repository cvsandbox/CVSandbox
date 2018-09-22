/*
    Test application for testing different video source

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include <XLocalVideoDevice.hpp>
#include <XJpegHttpStream.hpp>
#include <XMjpegHttpStream.hpp>
#include <XFFmpegNetworkStream.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::DirectShow;
using namespace CVSandbox::Video::MJpeg;
using namespace CVSandbox::Video::FFmpeg;

// Video source listener callback interface
class VideoSourceListener : public IVideoSourceListener
{
public:
    virtual void OnNewImage( const std::shared_ptr<const XImage>& image );
    virtual void OnError( const std::string& errorMessage );

public:
    shared_ptr<IVideoSource> VideoSource;
};

static shared_ptr<IVideoSource> CreateDirectShowSource( );
static shared_ptr<IVideoSource> CreateMJpegSource( );
static shared_ptr<IVideoSource> CreateFFmpegSource( );

int main( int argc, char* argv[] )
{
    // _CrtSetBreakAlloc(229);

    printf( "Testing video ...\r\n" );
    {
        // create video source
        shared_ptr<IVideoSource> device = CreateDirectShowSource( );

        if ( !device )
        {
            printf( "Failed creating video source \n" );
        }
        else
        {
            // configure listener
            VideoSourceListener videoSourceListener;
            videoSourceListener.VideoSource = device;
            device->SetListener( &videoSourceListener );

            // start video source
            device->Start( );

            for ( int i = 0; i < 10; i++ )
            {
                printf( "Is Running: %d \n", static_cast<int>( device->IsRunning( ) ) );

                if ( !device->IsRunning( ) )
                    break;

                Sleep( 500 );
            }

            printf( "Is Running: %d \n", static_cast<int>( device->IsRunning( ) ) );

            device->SignalToStop( );
            device->WaitForStop( );

            printf( "Video source has stopped \n" );
            printf( "Got %u frames \n", device->FramesReceived( ) );
            printf( "Is Running: %d \r\n", static_cast<int>( device->IsRunning( ) ) );
        }
    }

    printf( "\n\nDone\n" );
    getchar( );

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
    #endif

    return 0;
}

// Video source provides an new image to process
void VideoSourceListener::OnNewImage( const shared_ptr<const XImage>& image )
{
    printf( "Got image (#%u) ... ", VideoSource->FramesReceived( ) );

    if ( !image )
    {
        printf( "null image" );
    }
    else
    {
        printf( "Size: %dx%d", image->Width( ), image->Height( ) );
    }

    printf( "\n" );
}

// An error occurred in the video source
void VideoSourceListener::OnError( const string& errorMessage )
{
    printf( "! Error: %s \n", errorMessage.c_str( ) );
}

// Create Direct Show video source using the first available device
shared_ptr<IVideoSource> CreateDirectShowSource( )
{
    shared_ptr<XLocalVideoDevice> device;

    // get local available devices
    const vector<XDeviceName> devices = XLocalVideoDevice::GetAvailableDevices( );

    if ( devices.size( ) == 0 )
    {
        printf( "No local video devices found \n" );
    }
    else
    {
        int i = 0;

        // list found devices
        for ( vector<XDeviceName>::const_iterator it = devices.begin( ); it != devices.end( ); it++ )
        {
            printf( "Name: %s \n%s \n\n", it->Name( ).c_str( ), it->Moniker( ).c_str( ) );
        }

        // create first available device
        device = XLocalVideoDevice::Create( devices[0].Moniker( ) );

        // check capabilities of the device
        const vector<XDeviceCapabilities> capabilities = device->GetCapabilities( );
        const vector<XDevicePinInfo>      videoPins    = device->GetInputVideoPins( );

        printf( "Available resolutions: %d \n", static_cast<int>( capabilities.size( ) ) );
        for ( vector<XDeviceCapabilities>::const_iterator it = capabilities.begin( ); it != capabilities.end( ); it++ )
        {
            printf( "%d : %dx%d x %d @ Rate( avg = %d, max = %d, min = %d ) \n", i++,
                it->Width( ), it->Height( ), it->BitCount( ),
                it->AverageFrameRate( ), it->MaximumFrameRate( ), it->MinimumFrameRate( ) );
        }

        for ( vector<XDevicePinInfo>::const_iterator it = videoPins.begin( ); it != videoPins.end( ); it++ )
        {
            printf( "> input #: %d, type: %d \n", it->Index( ), it->Type( ) );
        }

        printf( "\n" );
    }

    return device;
}

shared_ptr<IVideoSource> CreateMJpegSource( )
{
    shared_ptr<XMjpegHttpStream> device;

    // test JPEG video source
    //shared_ptr<XJpegHttpStream> device = XJpegHttpStream::Create( "http://172.20.200.147/cgi/jpg/image.cgi" );
    //shared_ptr<XMjpegHttpStream> device = XMjpegHttpStream::Create( "http://172.20.200.147/cgi/mjpg/mjpg.cgi" );
    //shared_ptr<XJpegHttpStream> device = XJpegHttpStream::Create( "http://172.20.200.148/cgi/jpg/image.cgi" );
    //shared_ptr<XJpegHttpStream> device = XJpegHttpStream::Create( "http://en.wikipedia.org/wiki/List_of_HTTP_status_codes" );

    return device;
}

shared_ptr<IVideoSource> CreateFFmpegSource( )
{
    shared_ptr<XFFmpegNetworkStream> device;

    //shared_ptr<XFFmpegNetworkStream> device = XFFmpegNetworkStream::Create( "rtsp://129.16.208.82:554/mpeg4/media.amp" );
    //shared_ptr<XFFmpegNetworkStream> device = XFFmpegNetworkStream::Create( "rtsp://3tipcamdemo.dyndns.tv:554/av0_1&user=admin&password=admin" );
    //shared_ptr<XFFmpegNetworkStream> device = XFFmpegNetworkStream::Create( "rtsp://3tipcamdemo.dyndns.tv:554/av0_1&user=admin&password=admin" );
    //shared_ptr<XFFmpegNetworkStream> device = XFFmpegNetworkStream::Create( "rtsp://nhacuatui.dyndns.info/av0_0&user=admin&password=admin" );

    return device;
}
