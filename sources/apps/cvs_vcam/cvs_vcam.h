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

#pragma once
#ifndef CVS_VCAM_H
#define CVS_VCAM_H

namespace Private
{
    class CCVSVirtualCameraPinData;
}

class CCVSVirtualCameraPin : public CSourceStream, public IAMStreamConfig, public IKsPropertySet
{
public:
    CCVSVirtualCameraPin( HRESULT* phr, CSource* pFilter);
    ~CCVSVirtualCameraPin( );

    HRESULT GetMediaType( int iPosition, CMediaType* pMediaType );
    HRESULT CheckMediaType( const CMediaType* pmt );

    HRESULT DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest );
    HRESULT FillBuffer( IMediaSample* pSample);
    STDMETHODIMP Notify( IBaseFilter* pSelf, Quality q );

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP QueryInterface( REFIID riid, void** ppv );
    STDMETHODIMP_( ULONG ) AddRef( );
    STDMETHODIMP_( ULONG ) Release( );

    //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat( AM_MEDIA_TYPE* pmt );
    HRESULT STDMETHODCALLTYPE GetFormat( AM_MEDIA_TYPE** ppmt );
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities( int* piCount, int* piSize );
    HRESULT STDMETHODCALLTYPE GetStreamCaps( int index, AM_MEDIA_TYPE** pmt, BYTE* pSCC );

    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set( REFGUID guidPropSet, DWORD dwID, void* pInstanceData, DWORD cbInstanceData, void* pPropData, DWORD cbPropData );
    HRESULT STDMETHODCALLTYPE Get( REFGUID guidPropSet, DWORD dwPropID, void* pInstanceData, DWORD cbInstanceData, void* pPropData, DWORD cbPropData, DWORD* pcbReturned );
    HRESULT STDMETHODCALLTYPE QuerySupported( REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport );

private:
    Private::CCVSVirtualCameraPinData* mData;
};


class CVSVirtualCamera : public CSource
{
private:
    // Constructor is private because you have to use CreateInstance
    CVSVirtualCamera( IUnknown* pUnk, HRESULT* phr);
    ~CVSVirtualCamera( );


public:
    static CUnknown* WINAPI CreateInstance( IUnknown* pUnk, HRESULT* phr);

    IFilterGraph* GetGraph( ) { return m_pGraph; }

private:
    CCVSVirtualCameraPin* m_pPin;
};

#endif // CVS_VCAM_H
