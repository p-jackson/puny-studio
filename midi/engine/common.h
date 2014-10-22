//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once
#include <Windows.h>
#include <wrl\implements.h>
#include <mmreg.h>
#include <mfapi.h>

using namespace Microsoft::WRL;


// release and zero out a possible NULL pointer. note this will
// do the release on a temp copy to avoid reentrancy issues that can result from
// callbacks durring the release
template <class T> void SafeRelease(__deref_inout_opt T **ppT) {
  T *pTTemp = *ppT;    // temp copy
  *ppT = nullptr;      // zero the input
  if (pTTemp) {
    pTTemp->Release();
  }
}

// Exception Helper Method
inline void ThrowIfFailed(HRESULT hr) {
  if (FAILED(hr)) {
    // Set a breakpoint on this line to catch API errors.
    throw Platform::Exception::CreateException(hr);
  }
}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { SafeRelease(&x); }
#endif


#ifndef METHODASYNCCALLBACK
#define METHODASYNCCALLBACK(Parent, AsyncCallback, pfnCallback) \
class Callback##AsyncCallback :\
    public IMFAsyncCallback \
{ \
public: \
    Callback##AsyncCallback() : \
        _parent(((Parent*)((BYTE*)this - offsetof(Parent, m_x##AsyncCallback)))), \
        _dwQueueID( MFASYNC_CALLBACK_QUEUE_MULTITHREADED ) \
    { \
    } \
\
    STDMETHOD_( ULONG, AddRef )() \
    { \
        return _parent->AddRef(); \
    } \
    STDMETHOD_( ULONG, Release )() \
    { \
        return _parent->Release(); \
    } \
    STDMETHOD( QueryInterface )( REFIID riid, void **ppvObject ) \
    { \
        if (riid == IID_IMFAsyncCallback || riid == IID_IUnknown) \
                { \
            (*ppvObject) = this; \
            AddRef(); \
            return S_OK; \
                } \
        *ppvObject = NULL; \
        return E_NOINTERFACE; \
    } \
    STDMETHOD( GetParameters )( \
        /* [out] */ __RPC__out DWORD *pdwFlags, \
        /* [out] */ __RPC__out DWORD *pdwQueue) \
    { \
        *pdwFlags = 0; \
        *pdwQueue = _dwQueueID; \
        return S_OK; \
    } \
    STDMETHOD( Invoke )( /* [out] */ __RPC__out IMFAsyncResult * pResult ) \
    { \
        _parent->pfnCallback( pResult ); \
        return S_OK; \
    } \
    void SetQueueID( DWORD dwQueueID ) { _dwQueueID = dwQueueID; } \
\
protected: \
    Parent* _parent; \
    DWORD   _dwQueueID; \
           \
} m_x##AsyncCallback;
#endif



