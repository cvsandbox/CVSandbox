/*
    Test application for writing video files using FFmpeg

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
#include <ximaging.h>

#include <XFFmpegVideoFileWriter.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video::FFmpeg;

const int VideoFrameWidth    = 320;
const int VideoFrameHeight   = 240;
const int VideoFramesToWrite = 100;

static void WriteVideoFile( const shared_ptr<XFFmpegVideoFileWriter>& fileWriter, const string& fileName, XFFmpegVideoFileWriter::Codec codec );

int main( int /* argc */, char** /* argv */ )
{
    printf( "Testing video ...\r\n" );

    {
        shared_ptr<XFFmpegVideoFileWriter> fileWriter = XFFmpegVideoFileWriter::Create( );

        for ( int i = -1; i < static_cast<int>( XFFmpegVideoFileWriter::Codec::LastValue ) ; i++ )
        {
            char fileName[100];

            sprintf( fileName, "test (%d).avi", i );
            printf( "Writing file : %s ... \n", fileName );

            WriteVideoFile( fileWriter, fileName, static_cast<XFFmpegVideoFileWriter::Codec>( i ) );
        }
    }

    printf( "\nDone\n" );
    getchar( );

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks( );
    #endif
    
    return 0;
}

// Writes a test video file with RGB diagonal line and frame counter
void WriteVideoFile( const shared_ptr<XFFmpegVideoFileWriter>& fileWriter, const string& fileName, XFFmpegVideoFileWriter::Codec codec )
{
    shared_ptr<XImage> image     = XImage::Allocate( VideoFrameWidth, VideoFrameHeight, XPixelFormatRGB24 );
    XErrorCode         errorCode = fileWriter->Open( fileName, VideoFrameWidth, VideoFrameHeight, 30, codec, 500000 );
    char               buffer[16];

    if ( errorCode != SuccessCode )
    {
        printf( "Error: failed opening video file, error code : %d \n", errorCode );
    }
    else
    {
        int x[]  = { 0, 0, VideoFrameWidth - 1, VideoFrameWidth - 1 };
        int y[]  = { 0, VideoFrameHeight - 1, 0, VideoFrameHeight - 1 };
        int dx[] = { 1,  1, -1, -1 };
        int dy[] = { 1, -1,  1, -1 };

        for ( int i = 0; i < VideoFramesToWrite; i++ )
        {
            for ( int j = 0; j < 4; j++ )
            {
                uint8_t* ptr = image->Data( ) + image->Stride( ) * y[j] + x[j] * 3;

                ptr[RedIndex]      = 0;
                ptr[GreenIndex]    = 0;
                ptr[BlueIndex]     = 0;
                ptr[( i + j ) % 3] = 255;

                x[j] += dx[j];
                y[j] += dy[j];

                if ( x[j] >= VideoFrameWidth )
                {
                    dx[j] = -1;
                    x[j]  = VideoFrameWidth - 1;
                }
                if ( x[j] < 0 )
                {
                    dx[j] = 1;
                    x[j]  = 0;
                }
                if ( y[j] >= VideoFrameHeight )
                {
                    dy[j] = -1;
                    y[j]  = VideoFrameHeight - 1;
                }
                if ( y[j] < 0 )
                {
                    dy[j] = 1;
                    y[j]  = 0;
                }
            }

            sprintf( buffer, "%d", i + 1 );
            XDrawingText( image->ImageData( ), buffer, 0, 0, { 0xFFFFFFFF }, { 0xFF000000 }, true );

            errorCode = fileWriter->WriteVideFrame( image );
            if ( errorCode != SuccessCode )
            {
                printf( "Error: Failed writing frame %d, error code : %d \n", i, errorCode );
                break;
            }
        }

        fileWriter->Close( );
    }
}
