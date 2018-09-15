/*
    Test application for reading video files using FFmpeg

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
#include <ximaging_formats.h>
#include <XFFmpegVideoFileWriter.hpp>
#include <XFFmpegVideoFileReader.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video::FFmpeg;

static void ReadVideoFile( const shared_ptr<XFFmpegVideoFileReader>& fileWriter, const string& fileName );

int main( int /* argc */, char** /* argv */ )
{
    printf( "Testing video reading ...\r\n" );

    {
        shared_ptr<XFFmpegVideoFileReader> reader = XFFmpegVideoFileReader::Create( );

        for ( int i = -1; i < static_cast<int>( XFFmpegVideoFileWriter::Codec::LastValue ); i++ )
        {
            char fileName[100];

            sprintf( fileName, "test (%d).avi", i );

            ReadVideoFile( reader, fileName );
        }
    }

    printf( "\nDone\n" );
    getchar( );

#ifdef _MSC_VER
    _CrtDumpMemoryLeaks( );
#endif

    return 0;
}

// Read the specified video file and count number of frames actually read
void ReadVideoFile( const shared_ptr<XFFmpegVideoFileReader>& fileReader, const string& fileName )
{
    XErrorCode ecode = fileReader->Open( fileName );

    if ( ecode != SuccessCode )
    {
        printf( "Failed opening video file: %s, ecode: %d \n", fileName.c_str( ), ecode );
    }
    else
    {
        printf( "Video file opened, %s, %d x %d @ %f, frames = %d \n",
                fileReader->CodecLongName( ).c_str( ),
                fileReader->FrameSize( ).Width( ), fileReader->FrameSize( ).Height( ),
                fileReader->FrameRate( ), static_cast< int >( fileReader->FramesTotal( ) ) );
    }

    shared_ptr<XImage> image;
    int                counter = 0;
    char               imageFileName[100];

    while ( fileReader->GetNextFrame( image ) == SuccessCode )
    {
        counter++;

        if ( counter == 1 )
        {
            sprintf( imageFileName, "%s.%d.png", fileName.c_str( ), counter );
            XEncodePng( imageFileName, image->ImageData( ) );
        }
    }

    if ( counter != 0 )
    {
        sprintf( imageFileName, "%s.%d.png", fileName.c_str( ), counter );
        XEncodePng( imageFileName, image->ImageData( ) );
    }

    printf( "Got %d frames \n", counter );

    fileReader->Close( );
}
