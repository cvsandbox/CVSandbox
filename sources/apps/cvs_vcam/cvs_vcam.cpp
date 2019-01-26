/*
    Virtual Camera DirectShow filter for Computer Vision Sandbox

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

#include <streams.h>
#include <strsafe.h>
#include <windows.h>
#include <stdint.h>
#include <xtypes.h>
#include <ximaging.h>

#include "cvs_vcam.h"
#include "cvs_vcam_guids.h"

using namespace std;

typedef struct
{
    int Width;
    int Height;
    int Fps;
}
VideoResolution;

const static VideoResolution SupportedRosulutions[] =
{
    {  640,  480, 30 },
    {  800,  600, 30 },
    {  480,  360, 30 },
    {  320,  240, 30 },
    {  160,  120, 30 },
    { 1200,  900, 30 },
    { 1280,  720, 30 },
    { 1800, 1350, 30 },
    { 1920, 1080, 30 }
};

static const uint32_t   VCAM_MAGIC              = 0xFEEDBABE;

static const uint32_t   VCAM_BPP                = 24;
static const uint32_t   VCAM_BYTES_PP           = 3;
static const uint32_t   VCAM_MAX_WIDTH          = 2000;
static const uint32_t   VCAM_MAX_HEIGHT         = 1500;

static const uint32_t   VCAM_WIDTH_OFFSET       = VCAM_MAX_WIDTH * VCAM_MAX_HEIGHT * VCAM_BYTES_PP;
static const uint32_t   VCAM_HEIGHT_OFFSET      = VCAM_WIDTH_OFFSET  + sizeof( uint32_t );
static const uint32_t   VCAM_IMAGE_COUNTER      = VCAM_HEIGHT_OFFSET + sizeof( uint32_t );
static const uint32_t   VCAM_MAGIC_OFFSET       = VCAM_IMAGE_COUNTER + sizeof( uint32_t );
static const uint32_t   VCAM_BUF_SIZE           = VCAM_MAGIC_OFFSET  + sizeof( uint32_t );

static const LPCTSTR    VCAM_PIN_NAME           = NAME( "CVS VCAM OUT PIN" );
static const LPCTSTR    VCAM_SOURCE_NAME        = NAME( "CVSandboxVirtualCamera" );

static const LPCWSTR    VCAM_SHARED_MEMORY_NAME = L"CVS.VCAM.MEMORY";
static const LPCWSTR    VCAM_SHARED_MUTEX_NAME  = L"CVS.VCAM.MUTEX";

/**********************************************
*
*  Private data of CCVSVirtualCameraPin Class
*
**********************************************/

namespace Private
{
    class CCVSVirtualCameraPinData
    {
    public:
        CCVSVirtualCameraPinData( CSource* pParentFilter ) :
            FrameNumber( 0 ),
            ParentSourceFilter( pParentFilter ),
            TimerHandle( NULL ),
            RequestedFrameInterval( UNITS / 30 ),
            mutexHandle( NULL ), mapFileHandle( NULL ), sharedBuffer( nullptr ),
            resizedImage( nullptr ), lastResizeId( 0 )
        {
            TimerHandle = CreateWaitableTimer( NULL, FALSE, NULL );
        }

        ~CCVSVirtualCameraPinData( )
        {
            if ( TimerHandle != NULL )
            {
                CancelWaitableTimer( TimerHandle );
                CloseHandle( TimerHandle );
            }

            ResetSharedMemory( );
        }

        bool GetPushedImage( uint8_t* dstBuffer, int dstWidth, int dstHeight, int dstbitCount, int dstStride );

    private:
        void ResetSharedMemory( );
        bool IsSharedMemoryInitialized( );
        bool InitializeSharedMemory( );

    public:
        CCritSec        SharedState;            // Protects our internal state

        uint32_t        FrameNumber;
        CSource*        ParentSourceFilter;
        HANDLE          TimerHandle;

        REFERENCE_TIME  RequestedFrameInterval;

    private:
        HANDLE          mutexHandle;
        HANDLE          mapFileHandle;
        uint8_t*        sharedBuffer;

        ximage*         resizedImage;
        uint32_t        lastResizeId;
    };
}


/**********************************************
 *
 *  CCVSVirtualCameraPin Class
 *
 **********************************************/

CCVSVirtualCameraPin::CCVSVirtualCameraPin( HRESULT* phr, CSource* pFilter ) :
    CSourceStream( VCAM_PIN_NAME, phr, pFilter, L"Out" ),
    mData( new Private::CCVSVirtualCameraPinData( pFilter ) )
{
}

CCVSVirtualCameraPin::~CCVSVirtualCameraPin( )
{
    delete mData;
}

// GetMediaType: This method tells the downstream pin what types we support.

// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when (a) our filter proposes a media type, (b) the other filter proposes a
// type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int,CMediaType*) AND CheckMediaType.
//
// In this case we support more than one type.

HRESULT CCVSVirtualCameraPin::GetMediaType( int iPosition, CMediaType* pMediaType )
{
    HRESULT     ret = S_OK;
    CAutoLock   cAutoLock( m_pFilter->pStateLock( ) );

    if ( pMediaType == nullptr )
    {
        ret = E_POINTER;
    }
    else if ( iPosition < 0 )
    {
        ret = E_INVALIDARG;
    }
    else if ( iPosition >= XARRAY_SIZE( SupportedRosulutions ) )
    {
        ret = VFW_S_NO_MORE_ITEMS;
    }
    else
    {
        // allocate enough room for the VIDEOINFOHEADER and the color tables
        VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*) pMediaType->AllocFormatBuffer( sizeof( VIDEOINFOHEADER ) );
        VideoResolution  res = SupportedRosulutions[iPosition];

        if ( pvi == nullptr )
        {
            ret = E_OUTOFMEMORY;
        }
        else
        {
            ZeroMemory( pvi, pMediaType->cbFormat );

            pvi->AvgTimePerFrame          = UNITS / res.Fps;

            pvi->bmiHeader.biCompression  = BI_RGB;
            pvi->bmiHeader.biBitCount     = VCAM_BPP;
            pvi->bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
            pvi->bmiHeader.biWidth        = res.Width;
            pvi->bmiHeader.biHeight       = res.Height;
            pvi->bmiHeader.biPlanes       = 1;
            pvi->bmiHeader.biSizeImage    = GetBitmapSize( &pvi->bmiHeader );
            pvi->bmiHeader.biClrImportant = 0;

            // clear source and target rectangles
            SetRectEmpty( &( pvi->rcSource ) ); // we want the whole image area rendered
            SetRectEmpty( &( pvi->rcTarget ) ); // no particular destination rectangle

            pMediaType->SetType( &MEDIATYPE_Video );
            pMediaType->SetFormatType( &FORMAT_VideoInfo );
            pMediaType->SetTemporalCompression( FALSE );

            // wWork out the GUID for the subtype from the header info
            const GUID SubTypeGUID = GetBitmapSubtype( &pvi->bmiHeader );
            pMediaType->SetSubtype( &SubTypeGUID );

            pMediaType->SetSampleSize( pvi->bmiHeader.biSizeImage );
        }
    }

    return ret;
}

HRESULT CCVSVirtualCameraPin::CheckMediaType( const CMediaType* pmt )
{
    HRESULT ret = S_FALSE;

    if ( IsEqualGUID ( *pmt->Type( ), MEDIATYPE_Video ) )
    {
        if ( IsEqualGUID( *pmt->FormatType( ), FORMAT_VideoInfo ) )
        {
            VIDEOINFOHEADER* pvi    = (VIDEOINFOHEADER*) pmt->Format( );
            VIDEOINFOHEADER* pviSet = (VIDEOINFOHEADER*) m_mt.Format( );
            
            if ( ( ( pviSet != nullptr ) &&
                   ( pvi->bmiHeader.biCompression == pviSet->bmiHeader.biCompression ) &&
                   ( pvi->bmiHeader.biPlanes      == pviSet->bmiHeader.biPlanes      ) &&
                   ( pvi->bmiHeader.biBitCount    == pviSet->bmiHeader.biBitCount    ) &&
                   ( pvi->bmiHeader.biWidth       == pviSet->bmiHeader.biWidth       ) &&
                   ( pvi->bmiHeader.biHeight      == pviSet->bmiHeader.biHeight      ) )
                   ||
                 ( ( pviSet == nullptr ) &&
                   ( pvi->bmiHeader.biCompression == BI_RGB   ) &&
                   ( pvi->bmiHeader.biPlanes      == 1        ) &&
                   ( pvi->bmiHeader.biBitCount    == VCAM_BPP ) &&
                   ( pvi->bmiHeader.biWidth       == SupportedRosulutions[0].Width  ) &&
                   ( pvi->bmiHeader.biHeight      == SupportedRosulutions[0].Height ) ) )
            {
                ret = S_OK;
            }
        }
    }

    return ret;
}

HRESULT CCVSVirtualCameraPin::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest)
{
    HRESULT     ret = S_OK;
    CAutoLock   cAutoLock( m_pFilter->pStateLock( ) );

    if ( ( pAlloc == nullptr ) || ( pRequest == nullptr ) )
    {
        ret = E_POINTER;
    }
    else
    {
        VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*) m_mt.Format( );

        // ensure a minimum number of buffers
        if ( pRequest->cBuffers == 0 )
        {
            pRequest->cBuffers = 2;
        }

        if ( pvi != nullptr )
        {
            pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;
        }
        else
        {
            // default to the first supported resolution
            pRequest->cbBuffer = WIDTHBYTES( SupportedRosulutions[0].Width * VCAM_BPP ) * SupportedRosulutions[0].Height;
        }

        ALLOCATOR_PROPERTIES allocPropertie;
        ret = pAlloc->SetProperties( pRequest, &allocPropertie );
        if ( SUCCEEDED( ret ) )
        {
            // is this allocator unsuitable ?
            if ( allocPropertie.cbBuffer < pRequest->cbBuffer )
            {
                ret = E_FAIL;
            }
        }
    }

    return ret;
}

// Fill the stream's buffer with our frames
HRESULT CCVSVirtualCameraPin::FillBuffer( IMediaSample* pSample )
{
    HRESULT ret = S_OK;

    if ( pSample == nullptr )
    {
        ret = E_POINTER;
    }
    else
    {
        CAutoLock cAutoLockShared( &mData->SharedState );
        BYTE*     pData;
        long      cbData;

        if ( mData->TimerHandle != NULL )
        {
            if ( mData->FrameNumber == 0 )
            {
                // set timer on the first image request
                LARGE_INTEGER liDueTime;

                liDueTime.QuadPart = mData->RequestedFrameInterval;

                if ( !SetWaitableTimer( mData->TimerHandle, &liDueTime, static_cast<LONG>( mData->RequestedFrameInterval / 10000 ), NULL, NULL, 0 ) )
                {
                    CloseHandle( mData->TimerHandle );
                    mData->TimerHandle = NULL;
                }
            }
            else
            {
                // wait for time on subsequent image requests
                WaitForSingleObject( mData->TimerHandle, static_cast<DWORD>( mData->RequestedFrameInterval / 10000 ) );
            }
        }

        // access the sample's data buffer
        pSample->GetPointer( &pData );
        cbData = pSample->GetSize( );

        // check that we're still using video
        ASSERT( m_mt.formattype == FORMAT_VideoInfo );

        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) m_mt.pbFormat;

        int width    = pVih->bmiHeader.biWidth;
        int height   = pVih->bmiHeader.biHeight;
        int bitCount = pVih->bmiHeader.biBitCount;
        int stride   = ( ( ( ( width * bitCount ) + 31 ) & ~31 ) >> 3 );

        if ( !mData->GetPushedImage( pData, width, height, bitCount, stride ) )
        {
            for ( int y = 0; y < height; y++ )
            {
                BYTE* pLine = pData + y * stride;

                for ( int x = 0; x < width; x++, pLine += 3 )
                {
                    BYTE v = (BYTE) ( rand( ) % 255 );

                    pLine[0] = pLine[1] = pLine[2] = v;
                }
            }
        }

        // set the timestamps that will govern playback frame rate
        REFERENCE_TIME rtStart = mData->FrameNumber * mData->RequestedFrameInterval;
        REFERENCE_TIME rtStop  = rtStart + mData->RequestedFrameInterval;

        pSample->SetTime( &rtStart, &rtStop );
        mData->FrameNumber++;

        // set TRUE on every sample for uncompressed frames
        pSample->SetSyncPoint( TRUE );
    }

    return ret;
}

// Not implemented. If the file-writing filter slows the graph down, we just do nothing, which means
// wait until we're unblocked.
STDMETHODIMP CCVSVirtualCameraPin::Notify( IBaseFilter*, Quality )
{
    return E_FAIL;
}


namespace Private
{

// Free both shared mutex handle and memory handle
void CCVSVirtualCameraPinData::ResetSharedMemory( )
{
    if ( sharedBuffer != nullptr )
    {
        UnmapViewOfFile( sharedBuffer );
        sharedBuffer = nullptr;
    }

    if ( mapFileHandle != NULL )
    {
        CloseHandle( mapFileHandle );
        mapFileHandle = NULL;
    }

    if ( mutexHandle != NULL )
    {
        CloseHandle( mutexHandle );
        mutexHandle = NULL;
    }

    XImageFree( &resizedImage );
}

// Check if initialization was done successfully
bool CCVSVirtualCameraPinData::IsSharedMemoryInitialized( )
{
    return ( sharedBuffer != nullptr );
}

// Initialize shared mutex and memory
bool CCVSVirtualCameraPinData::InitializeSharedMemory( )
{
    bool ret = false;

    lastResizeId = 0;

    mutexHandle  = OpenMutex( SYNCHRONIZE, FALSE, VCAM_SHARED_MUTEX_NAME );

    if ( mutexHandle != NULL )
    {
        mapFileHandle = OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            VCAM_SHARED_MEMORY_NAME );

        if ( mapFileHandle != NULL )
        {
            sharedBuffer = static_cast<uint8_t*>( MapViewOfFile( mapFileHandle, FILE_MAP_ALL_ACCESS, 0, 0, VCAM_BUF_SIZE ) );

            if ( sharedBuffer != nullptr )
            {
                ret = true;
            }
        }
    }

    if ( !ret )
    {
        ResetSharedMemory( );
    }

    return ret;
}

// Get pushed image from shared memory
bool CCVSVirtualCameraPinData::GetPushedImage( uint8_t* dstBuffer, int dstWidth, int dstHeight, int dstBitCount, int dstStride )
{
    bool ret = false;

    if ( !IsSharedMemoryInitialized( ) )
    {
        InitializeSharedMemory( );
    }

    if ( IsSharedMemoryInitialized( ) )
    {
        if ( WaitForSingleObject( mutexHandle, INFINITE ) == WAIT_OBJECT_0 )
        {
            if ( ( *( (uint32_t*)( sharedBuffer + VCAM_MAGIC_OFFSET ) ) == VCAM_MAGIC ) &&
                 ( dstBitCount == VCAM_BPP ) )
            {
                int      srcWidth  = static_cast<int>( *( (uint32_t*) ( sharedBuffer + VCAM_WIDTH_OFFSET  ) ) );
                int      srcHeight = static_cast<int>( *( (uint32_t*) ( sharedBuffer + VCAM_HEIGHT_OFFSET ) ) );
                int      srcStride = srcWidth * VCAM_BYTES_PP;
                uint8_t* srcBuffer = nullptr;

                if ( ( srcWidth == dstWidth ) && ( srcHeight == dstHeight ) )
                {
                    srcBuffer = sharedBuffer;
                }
                else
                {
                    if ( *( (uint32_t*) ( sharedBuffer + VCAM_IMAGE_COUNTER ) ) != lastResizeId )
                    {
                        if ( XImageAllocateRaw( dstWidth, dstHeight, XPixelFormatRGB24, &resizedImage ) == SuccessCode )
                        {
                            ximage* srcImage = nullptr;

                            if ( XImageCreate( sharedBuffer, srcWidth, srcHeight, srcStride, XPixelFormatRGB24, &srcImage ) == SuccessCode )
                            {
                                if ( ResizeImageBilinear( srcImage, resizedImage ) == SuccessCode )
                                {
                                    lastResizeId = *( (uint32_t*) ( sharedBuffer + VCAM_IMAGE_COUNTER ) );
                                    srcBuffer = resizedImage->data;
                                    srcStride = resizedImage->stride;
                                }
                            }

                            XImageFree( &srcImage );
                        }
                    }
                    else
                    {
                        srcBuffer = resizedImage->data;
                        srcStride = resizedImage->stride;
                    }
                }

                if ( srcBuffer != nullptr )
                {
                    for ( int y = 0; y < dstHeight; y++ )
                    {
                        uint8_t* srcRow = srcBuffer + y * srcStride;
                        uint8_t* dstRow = dstBuffer + ( dstHeight - 1 - y ) * dstStride;

                        for ( int x = 0; x < dstWidth; x++, dstRow += 3, srcRow += 3 )
                        {
                            dstRow[0] = srcRow[BlueIndex];
                            dstRow[1] = srcRow[GreenIndex];
                            dstRow[2] = srcRow[RedIndex];
                        }
                    }

                    ret = true;
                }
            }

            ReleaseMutex( mutexHandle );
        }
    }
    
    return ret;
}

} // namespace Private


//////////////////////////////////////////////////////////////////////////
//  IUnknown
//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_( ULONG ) CCVSVirtualCameraPin::AddRef( )
{
    return GetOwner( )->AddRef( );
}

STDMETHODIMP_( ULONG ) CCVSVirtualCameraPin::Release( )
{
    return GetOwner( )->Release( );
}

HRESULT CCVSVirtualCameraPin::QueryInterface( REFIID riid, void** ppv )
{
    // Standard OLE stuff
    if ( riid == _uuidof( IAMStreamConfig ) )
    {
        *ppv = static_cast<IAMStreamConfig*>( this );
    }
    else if ( riid == _uuidof( IKsPropertySet ) )
    {
        *ppv = static_cast<IKsPropertySet*>( this );
    }
    else
    {
        return CSourceStream::QueryInterface( riid, ppv );
    }

    AddRef( );
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CCVSVirtualCameraPin::SetFormat( AM_MEDIA_TYPE* pmt )
{
    HRESULT ret = S_OK;

    if ( pmt == nullptr )
    {
        ret = E_POINTER;
    }
    else if ( ( !IsEqualGUID( pmt->majortype, MEDIATYPE_Video ) ) ||
              ( !IsEqualGUID( pmt->formattype, FORMAT_VideoInfo ) ) )
    {
        ret = VFW_E_INVALIDMEDIATYPE;
    }
    else
    {
        VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*) ( pmt->pbFormat );

        if ( ( pvi->bmiHeader.biCompression != BI_RGB ) || ( pvi->bmiHeader.biBitCount != VCAM_BPP ) )
        {
            ret = VFW_E_INVALIDMEDIATYPE;
        }
        else
        {
            int supportedResolutionsCount = XARRAY_SIZE( SupportedRosulutions );
            int i;

            for ( i = 0; i < supportedResolutionsCount; i++ )
            {
                if ( ( pvi->bmiHeader.biWidth  == SupportedRosulutions[i].Width  ) &&
                     ( pvi->bmiHeader.biHeight == SupportedRosulutions[i].Height ) )
                {
                    break;
                }
            }

            if ( i == supportedResolutionsCount )
            {
                ret = VFW_E_INVALIDMEDIATYPE;
            }
            else
            {
                IPin* pin = nullptr;

                m_mt = *pmt;
                mData->RequestedFrameInterval = ( (VIDEOINFOHEADER*) pmt->pbFormat )->AvgTimePerFrame;

                ConnectedTo( &pin );

                if ( pin != nullptr )
                {
                    IFilterGraph *pGraph = static_cast<CVSVirtualCamera*>( mData->ParentSourceFilter )->GetGraph( );
                    pGraph->Reconnect( this );
                }
            }
        }
    }

    return ret;
}

HRESULT STDMETHODCALLTYPE CCVSVirtualCameraPin::GetFormat( AM_MEDIA_TYPE** ppmt )
{
    HRESULT ret = E_POINTER;

    if ( ppmt != nullptr )
    {
        *ppmt = CreateMediaType( &m_mt );
        ret   = ( *ppmt == nullptr ) ? E_OUTOFMEMORY : S_OK;
    }

    return ret;
}

HRESULT STDMETHODCALLTYPE CCVSVirtualCameraPin::GetNumberOfCapabilities( int* piCount, int* piSize )
{
    HRESULT ret = E_POINTER;

    if ( ( piCount != nullptr ) && (piSize != nullptr ) )
    {
        *piCount = XARRAY_SIZE( SupportedRosulutions );
        *piSize  = sizeof( VIDEO_STREAM_CONFIG_CAPS );
        ret      = S_OK;
    }

    return ret;
}

HRESULT STDMETHODCALLTYPE CCVSVirtualCameraPin::GetStreamCaps( int index, AM_MEDIA_TYPE** pmt, BYTE* pSCC )
{
    HRESULT ret = E_POINTER;

    if ( ( index < 0 ) || ( index >=  XARRAY_SIZE( SupportedRosulutions ) ) )
    {
        ret = E_INVALIDARG;
    }
    else if ( ( pmt != nullptr ) && ( pSCC != nullptr ) )
    {
        AM_MEDIA_TYPE* mediaType = CreateMediaType( &m_mt );

        if ( mediaType == nullptr )
        {
            ret = E_OUTOFMEMORY;
        }
        else
        {
            mediaType->pbFormat = (BYTE*) CoTaskMemAlloc( sizeof( VIDEOINFOHEADER ) );

            if ( mediaType->pbFormat == nullptr )
            {
                DeleteMediaType( mediaType );
                ret = E_OUTOFMEMORY;
            }
            else
            {
                VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*) ( mediaType->pbFormat );
                VideoResolution  res = SupportedRosulutions[index];

                pvi->AvgTimePerFrame            = UNITS / res.Fps;

                pvi->bmiHeader.biCompression    = BI_RGB;
                pvi->bmiHeader.biBitCount       = VCAM_BPP;
                pvi->bmiHeader.biSize           = sizeof( BITMAPINFOHEADER );
                pvi->bmiHeader.biWidth          = res.Width;
                pvi->bmiHeader.biHeight         = res.Height;
                pvi->bmiHeader.biPlanes         = 1;
                pvi->bmiHeader.biSizeImage      = GetBitmapSize( &pvi->bmiHeader );
                pvi->bmiHeader.biClrImportant   = 0;

                SetRectEmpty( &( pvi->rcSource ) ); // we want the whole image area rendered.
                SetRectEmpty( &( pvi->rcTarget ) ); // no particular destination rectangle

                mediaType->majortype            = MEDIATYPE_Video;
                mediaType->subtype              = MEDIASUBTYPE_RGB24;
                mediaType->formattype           = FORMAT_VideoInfo;
                mediaType->bTemporalCompression = FALSE;
                mediaType->bFixedSizeSamples    = FALSE;
                mediaType->lSampleSize          = pvi->bmiHeader.biSizeImage;
                mediaType->cbFormat             = sizeof( VIDEOINFOHEADER );

                VIDEO_STREAM_CONFIG_CAPS* pvscc = (VIDEO_STREAM_CONFIG_CAPS*) pSCC;

                pvscc->guid                 = FORMAT_VideoInfo;
                pvscc->VideoStandard        = AnalogVideo_None;
                pvscc->InputSize.cx         = res.Width;
                pvscc->InputSize.cy         = res.Height;
                pvscc->MinCroppingSize.cx   = res.Width;
                pvscc->MinCroppingSize.cy   = res.Height;
                pvscc->MaxCroppingSize.cx   = res.Width;
                pvscc->MaxCroppingSize.cy   = res.Height;
                pvscc->CropGranularityX     = res.Width;
                pvscc->CropGranularityY     = res.Height;
                pvscc->CropAlignX           = 0;
                pvscc->CropAlignY           = 0;

                pvscc->MinOutputSize.cx     = res.Width;
                pvscc->MinOutputSize.cy     = res.Height;
                pvscc->MaxOutputSize.cx     = res.Width;
                pvscc->MaxOutputSize.cy     = res.Height;
                pvscc->OutputGranularityX   = 0;
                pvscc->OutputGranularityY   = 0;
                pvscc->StretchTapsX         = 0;
                pvscc->StretchTapsY         = 0;
                pvscc->ShrinkTapsX          = 0;
                pvscc->ShrinkTapsY          = 0;
                pvscc->MinFrameInterval     = UNITS / res.Fps;
                pvscc->MaxFrameInterval     = UNITS / 1;
                pvscc->MinBitsPerSecond     = res.Width * res.Height * VCAM_BPP * 1;
                pvscc->MaxBitsPerSecond     = res.Width * res.Height * VCAM_BPP * res.Fps;

                *pmt = mediaType;
                ret  = S_OK;
            }
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////

HRESULT CCVSVirtualCameraPin::Set( REFGUID guidPropSet, DWORD dwID, void* pInstanceData,
                                   DWORD cbInstanceData, void* pPropData, DWORD cbPropData )
{
    // cannot set any properties
    return E_NOTIMPL;
}

// Return the pin category (our only property)
HRESULT CCVSVirtualCameraPin::Get(
        REFGUID guidPropSet,        // Which property set
        DWORD   dwPropID,           // Which property in that set
        void*   pInstanceData,      // Instance data (ignore)
        DWORD   cbInstanceData,     // Size of the instance data (ignore)
        void*   pPropData,          // Buffer to receive the property data
        DWORD   cbPropData,         // Size of the buffer
        DWORD*  pcbReturned         // Return the size of the property
    )
{
    HRESULT ret = S_OK;

    if ( guidPropSet != AMPROPSETID_Pin )
    {
        ret = E_PROP_SET_UNSUPPORTED;
    }
    else if ( dwPropID != AMPROPERTY_PIN_CATEGORY )
    {
        ret = E_PROP_ID_UNSUPPORTED;
    }
    else if ( ( pPropData == NULL ) && ( pcbReturned == NULL ) )
    {
        ret = E_POINTER;
    }
    else
    {
        if ( pcbReturned )
        {
            *pcbReturned = sizeof( GUID );
        }

        if ( pPropData != NULL )
        {
            if ( cbPropData < sizeof( GUID ) )
            {
                ret = E_UNEXPECTED; // the buffer is too small
            }
            else
            {
                *(GUID *) pPropData = PIN_CATEGORY_CAPTURE;
            }
        }
    }

    return ret;
}

// Query whether the pin supports the specified property
HRESULT CCVSVirtualCameraPin::QuerySupported( REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport )
{
    HRESULT ret = S_OK;

    if ( guidPropSet != AMPROPSETID_Pin )
    {
        ret = E_PROP_SET_UNSUPPORTED;
    }
    else if ( dwPropID != AMPROPERTY_PIN_CATEGORY )
    {
        ret = E_PROP_ID_UNSUPPORTED;
    }
    else
    {
        // we support getting this property, but not setting it
        if ( pTypeSupport != nullptr )
        {
            *pTypeSupport = KSPROPERTY_SUPPORT_GET;
        }
    }

    return ret;
}


/**********************************************
 *
 *  CVSVirtualCamera Class
 *
 **********************************************/

CVSVirtualCamera::CVSVirtualCamera( IUnknown* pUnk, HRESULT* phr)
    : CSource( VCAM_SOURCE_NAME, pUnk, CLSID_CVSandboxVirtualCamera )
{
    // the pin magically adds itself to our pin array
    m_pPin = new CCVSVirtualCameraPin( phr, this );

    if ( phr != nullptr )
    {
        *phr = ( m_pPin == nullptr ) ? E_OUTOFMEMORY : S_OK;
    }  
}

CVSVirtualCamera::~CVSVirtualCamera( )
{
    delete m_pPin;
}

CUnknown* WINAPI CVSVirtualCamera::CreateInstance( IUnknown* pUnk, HRESULT* phr )
{
    CVSVirtualCamera* pNewFilter = new CVSVirtualCamera( pUnk, phr );

    if ( phr != nullptr )
    {    
        *phr = ( pNewFilter == nullptr ) ? E_OUTOFMEMORY : S_OK;
    }

    return pNewFilter;
}
