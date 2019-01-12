/*
    FFMPEG video writing plug-ins of Computer Vision Sandbox

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

#include "VideoFileWriterPlugin.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <XVariant.hpp>
#include <XInterfaces.hpp>
#include <XFFmpegVideoFileWriter.hpp>
#include <XThread.hpp>

#ifdef WIN32
    #include <windows.h>
#else
#endif

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Video::FFmpeg;
using namespace CVSandbox::Threading;

// List of supported pixel formats
const XPixelFormat VideoFileWriterPlugin::supportedPixelFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

namespace Private
{
    static const char* DEFAULT_BASE_FILE_NAME = "video";

    // Class describing file to clean-up when the corresponding option is enabled
    class FileData
    {
    public:
        FileData( const string& fileName, double sizeMb, uint64_t fileTime ) :
            FileName( fileName ), SizeMb( sizeMb ), FileTime( fileTime )
        {
        }

        bool operator<( const FileData& rhs ) const
        {
            return ( FileTime > rhs.FileTime );
        }

    public:
        string   FileName;
        double   SizeMb;

    private:
        uint64_t FileTime;
    };

    // Internals of video file writer class
    class VideoFileWriterPluginData : private Uncopyable
    {
    public:
        VideoFileWriterPluginData( ) : FolderToWrite( ), BaseFileName( DEFAULT_BASE_FILE_NAME ), BaseFullFileName( DEFAULT_BASE_FILE_NAME ),
            FrameRate( 30 ), BitRate( 500 ), SyncPresentationTime( false ), AppendTimeStampToFileName( false ), SplitVideoFiles( false ),
            FragmentLength( 60 ), RemoveOldFiles( false ), DirectorySizeLimit( 10000 ),
            Codec( XFFmpegVideoFileWriter::Codec::MPEG4 ),
            VideoWriter( XFFmpegVideoFileWriter::Create( ) ), FolderCleanupThread( ),
            VideoFileStartTime( ), LastCleanupTime( ), IsCleanupTimerStarted( false ), LastPresentationTime( -1 )
        {
        }

        void UpdateBaseFullFileName( );

        static void FolderCleanupThreadHandler( void* param );
        static vector<FileData> CollectFiles( const string& searchPattern );

    public:
        string      FolderToWrite;
        string      BaseFileName;
        string      BaseFullFileName;
        uint8_t     FrameRate;
        uint16_t    BitRate;
        bool        SyncPresentationTime;
        bool        AppendTimeStampToFileName;
        bool        SplitVideoFiles;
        uint8_t     FragmentLength;
        bool        RemoveOldFiles;
        uint32_t    DirectorySizeLimit;

        XFFmpegVideoFileWriter::Codec               Codec;
        const shared_ptr<XFFmpegVideoFileWriter>    VideoWriter;
        XThread                                     FolderCleanupThread;

        steady_clock::time_point                    VideoFileStartTime;
        steady_clock::time_point                    LastCleanupTime;
        bool                                        IsCleanupTimerStarted;
        int64_t                                     LastPresentationTime;
    };
}

VideoFileWriterPlugin::VideoFileWriterPlugin( ) :
    mData( new ::Private::VideoFileWriterPluginData( ) )
{
}

VideoFileWriterPlugin::~VideoFileWriterPlugin( )
{
    delete mData;
}

void VideoFileWriterPlugin::Dispose( )
{
    Reset( );

    if ( mData->FolderCleanupThread.IsRunning( ) )
    {
        mData->FolderCleanupThread.Join( );
    }

    delete this;
}

// Get specified property value of the plug-in
XErrorCode VideoFileWriterPlugin::GetProperty( int32_t id, xvariant* value ) const
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
        value->value.strVal = XStringAlloc( mData->BaseFileName.c_str( ) );
        break;

    case 2:
        value->type        = XVT_U1;
        value->value.ubVal = static_cast<uint8_t>( mData->Codec );
        break;

    case 3:
        value->type        = XVT_U2;
        value->value.usVal = mData->BitRate;
        break;

    case 4:
        value->type        = XVT_U1;
        value->value.ubVal = mData->FrameRate;
        break;

    case 5:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->SyncPresentationTime;
        break;

    case 6:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->AppendTimeStampToFileName;
        break;

    case 7:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->SplitVideoFiles;
        break;

    case 8:
        value->type        = XVT_U1;
        value->value.ubVal = mData->FragmentLength;
        break;

    case 9:
        value->type          = XVT_Bool;
        value->value.boolVal =  mData->RemoveOldFiles;
        break;

    case 10:
        value->type        = XVT_U4;
        value->value.uiVal = mData->DirectorySizeLimit;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode VideoFileWriterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = ErrorFailed;
    xvariant    convertedValue;

    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 11, &convertedValue );

    if ( ret == SuccessCode )
    {
        XVariant    xvar( convertedValue );

        switch ( id )
        {
        case 0:
            mData->FolderToWrite = xvar.ToString( );
            mData->UpdateBaseFullFileName( );
            break;

        case 1:
            mData->BaseFileName = xvar.ToString( );
            mData->UpdateBaseFullFileName( );
            break;

        case 2:
            mData->Codec = static_cast<XFFmpegVideoFileWriter::Codec>( XINRANGE( xvar.ToUByte( ), 0, static_cast<uint8_t>( XFFmpegVideoFileWriter::Codec::LastValue ) - 1 ) );
            break;

        case 3:
            mData->BitRate = XMAX( xvar.ToUShort( ), 50 );
            break;

        case 4:
            mData->FrameRate = XINRANGE( xvar.ToUByte( ), 1, 60 );
            break;

        case 5:
            mData->SyncPresentationTime = xvar.ToBool( );
            break;

        case 6:
            mData->AppendTimeStampToFileName = xvar.ToBool( );
            break;

        case 7:
            mData->SplitVideoFiles = xvar.ToBool( );
            if ( mData->SplitVideoFiles )
            {
                mData->AppendTimeStampToFileName = true;
            }
            break;

        case 8:
            mData->FragmentLength = XINRANGE( xvar.ToUByte( ), 5, 120 );
            break;

        case 9:
            mData->RemoveOldFiles = xvar.ToBool( );
            break;

        case 10:
            mData->DirectorySizeLimit = XMAX( xvar.ToUInt( ), 100 );
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
bool VideoFileWriterPlugin::IsReadOnlyMode( )
{
    return true;
}

// Get pixel formats supported by the video processing plug-in
XErrorCode VideoFileWriterPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode VideoFileWriterPlugin::ProcessImage( ximage* src )
{
    XErrorCode ret = ErrorFailedPluginInstantiation;

    if ( mData->VideoWriter )
    {
        if ( src == nullptr )
        {
            ret = ErrorNullParameter;
        }
        else
        {
            ret = SuccessCode;

            // check if current file should be closed
            if ( ( mData->SplitVideoFiles ) && ( mData->VideoWriter->IsOpen( ) ) )
            {
                auto currentLength = duration_cast<std::chrono::seconds>( steady_clock::now( ) - mData->VideoFileStartTime ).count( );

                if ( currentLength  > mData->FragmentLength * 60 )
                {
                    mData->VideoWriter->Close( );
                }
            }

            // create new video file if required
            if ( !mData->VideoWriter->IsOpen( ) )
            {
                string fileName = mData->BaseFullFileName;
                
                if ( mData->AppendTimeStampToFileName )
                {
                    time_t     timeNow    = time( 0 );
                    struct tm* localTime  = localtime( &timeNow );
                    char       buffer[32] = { 0 };

                    sprintf( buffer, " - %04d-%02d-%02d %02d-%02d-%02d",
                        localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
                        localTime->tm_hour, localTime->tm_min, localTime->tm_sec );

                    fileName += buffer;
                }
                
                fileName += ".avi";

                ret = mData->VideoWriter->Open( fileName, src->width, src->height, mData->FrameRate, mData->Codec, mData->BitRate * 1000 );
                mData->VideoFileStartTime = steady_clock::now( );
            }

            // write new video frame
            if ( ret == SuccessCode )
            {
                int64_t presentationTime = -1;

                if ( mData->SyncPresentationTime )
                {
                    if ( mData->LastPresentationTime != -1 )
                    {
                        auto timeSinceVideoStart = duration_cast<std::chrono::milliseconds>( steady_clock::now( ) - mData->VideoFileStartTime ).count( ) / 1000.0;

                        presentationTime = static_cast<int64_t>( timeSinceVideoStart * mData->FrameRate );

                        // if video source provides frames faster than the configured video FPS, we need to adjust presentation time,
                        // so no two frames end up with the same time
                        if ( presentationTime <= mData->LastPresentationTime )
                        {
                            presentationTime = mData->LastPresentationTime + 1;
                        }
                    }
                    else
                    {
                        presentationTime = 0;
                    }

                    mData->LastPresentationTime = presentationTime;
                }

                ret = mData->VideoWriter->WriteVideFrame( XImage::Create( src ), presentationTime );
            }

            // check if it is required to clean-up destination folder
            if ( mData->RemoveOldFiles )
            {
                if ( !mData->IsCleanupTimerStarted )
                {
                    mData->LastCleanupTime       = steady_clock::now( );
                    mData->IsCleanupTimerStarted = true;
                }
                else
                {
                    auto timeSinceLastCleanup = duration_cast<std::chrono::seconds>( steady_clock::now( ) - mData->LastCleanupTime ).count( );

                    // run clean-up routine every 5 minutes
                    if ( timeSinceLastCleanup > 60 * 5 )
                    {
                        if ( !mData->FolderCleanupThread.IsRunning( ) )
                        {
                            mData->FolderCleanupThread.Create( ::Private::VideoFileWriterPluginData::FolderCleanupThreadHandler, mData );
                        }

                        mData->LastCleanupTime = steady_clock::now( );
                    }
                }
            }
        }
    }

    return ret;
}

// Reset run time state of the video processing plug-in
void VideoFileWriterPlugin::Reset( )
{
    if ( mData->VideoWriter )
    {
        mData->VideoWriter->Close( );
    }

    mData->IsCleanupTimerStarted = false;
    mData->LastPresentationTime  = -1;
}

namespace Private
{
    // Prepare base full file name, which include folder name and file name without extension (and possible time stamp)
    void VideoFileWriterPluginData::UpdateBaseFullFileName( )
    {
        size_t length = FolderToWrite.length( );

        if ( length == 0 )
        {
            BaseFullFileName = BaseFileName;
        }
        else
        {
            BaseFullFileName = FolderToWrite;

            if ( ( BaseFullFileName[length - 1] != '/' ) && ( BaseFullFileName[length - 1] != '\\' ) )
            {
                BaseFullFileName += '/';
            }

            BaseFullFileName += BaseFileName;
        }

        length = BaseFullFileName.length( ) - 1;

        while ( ( length >= 0 ) && ( BaseFullFileName[length] == '.' ) )
        {
            BaseFullFileName.pop_back( );
            length = BaseFullFileName.length( ) - 1;
        }

        if ( length == 0 )
        {
            BaseFullFileName = DEFAULT_BASE_FILE_NAME;
        }
    }

    void VideoFileWriterPluginData::FolderCleanupThreadHandler( void* param )
    {
        VideoFileWriterPluginData* me = static_cast<VideoFileWriterPluginData*>( param );

        string folder    = me->FolderToWrite;
        double sizeLimit = static_cast<double>( me->DirectorySizeLimit );

        if ( ( !folder.empty( ) ) && ( folder.back( ) != '/' ) && ( folder.back( ) != '\\' ) )
        {
            folder += '/';
        }

        // collect video files
        vector<FileData> filesFound = CollectFiles( folder + "*.avi" );
        // sort them, so old files are at the end of the container
        sort( filesFound.begin( ), filesFound.end( ) );
        // get total size of the found files
        double totalSize = accumulate( filesFound.begin( ), filesFound.end( ), 0.0, [] ( double currentSum, const FileData& value )
        {
            return currentSum + value.SizeMb;
        } );

        // delete old files to get back into the limit
        while ( totalSize > sizeLimit )
        {
            auto fileInfo = filesFound.back( );

            remove( ( folder + fileInfo.FileName ).c_str( ) );
            totalSize -= fileInfo.SizeMb;

            filesFound.pop_back( );
        }
    }

    vector<FileData> VideoFileWriterPluginData::CollectFiles( const string& searchPattern )
    {
        vector<FileData> filesFound;

#ifdef WIN32
        HANDLE           hFind;
        WIN32_FIND_DATAA ffData;

        if ( ( hFind = FindFirstFileA( searchPattern.c_str( ), &ffData ) ) != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ( ( ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
                {
                    filesFound.push_back( FileData(
                        ffData.cFileName,
                        (double) ( ( (uint64_t) ffData.nFileSizeHigh << 32 ) + ffData.nFileSizeLow ) / ( 1024.0 * 1024.0 ),
                        ( (uint64_t) ffData.ftCreationTime.dwHighDateTime << 32 ) + ffData.ftCreationTime.dwLowDateTime
                        ) );
                }
            }
            while ( FindNextFileA( hFind, &ffData ) );

            FindClose( hFind );
        }
#else
        Need to implement it. Lets fail the build for now.
#endif

        return filesFound;
    }
}
