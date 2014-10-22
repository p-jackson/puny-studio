//
// AudioOutput.cpp
//
// WASAPI-based audio output class for real-time audio generation.
// Author: Gerry Beauregard, 2013
// 
// Loosely based on the MSDN WASAPI Sample here:
// http://code.msdn.microsoft.com/windowsapps/Windows-Audio-Session-22dcab6b
// 

#include "audio-output.h"
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

using namespace Windows::System::Threading;



// Behaves like printf, but sends output to Visual Studio Output console
static void DebugPrintf(const char *format, ...) {
  const int BUF_LEN = 4096;
  char buf[BUF_LEN];
  va_list args;
  va_start(args, format);
  vsprintf_s<BUF_LEN>(buf, format, args);
  va_end(args);
  OutputDebugStringA(buf);
}


typedef enum {
  SampleTypeUnknown,
  SampleTypeFloat,
  SampleType16BitPCM,
} RenderSampleType;

//
//  CalculateMixFormatType()
//
//  Determine IEEE Float or PCM samples based on media type
//
inline RenderSampleType CalculateMixFormatType(WAVEFORMATEX *wfx) {
  if ((wfx->wFormatTag == WAVE_FORMAT_PCM) ||
    ((wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) &&
    (reinterpret_cast<WAVEFORMATEXTENSIBLE *>(wfx)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM))) {
    if (wfx->wBitsPerSample == 16) {
      return RenderSampleType::SampleType16BitPCM;
    }
  }
  else if ((wfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
    ((wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) &&
    (reinterpret_cast<WAVEFORMATEXTENSIBLE *>(wfx)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))) {
    return RenderSampleType::SampleTypeFloat;
  }

  return RenderSampleType::SampleTypeUnknown;
}


//static const int NUM_CHANNELS = 2;
//static const double SAMPLE_RATE = 44100.0;


//
//  AudioOutput()
//
AudioOutput::AudioOutput() :
m_BufferFrames(0),
m_AudioClient(nullptr),
m_AudioRenderClient(nullptr),
m_SampleReadyAsyncResult(nullptr),
m_audioCallback(nullptr),
m_deviceState(DeviceState::DeviceStateUnInitialized),
m_hnsBufferDuration(0),
m_IsBackground(FALSE),
m_IsHWOffload(FALSE),
m_SampleReadyEvent(nullptr),
m_sampleRate(0),
m_numChannels(0) {
}

//
//  ~AudioOutput()
//
AudioOutput::~AudioOutput() {
  SAFE_RELEASE(m_AudioClient);
  SAFE_RELEASE(m_AudioRenderClient);
  SAFE_RELEASE(m_SampleReadyAsyncResult);

  if (INVALID_HANDLE_VALUE != m_SampleReadyEvent) {
    CloseHandle(m_SampleReadyEvent);
    m_SampleReadyEvent = INVALID_HANDLE_VALUE;
  }

  DeleteCriticalSection(&m_CritSec);
}

//
//  Init
//
//  Activates the default audio renderer on a asynchronous callback thread.  This needs
//  to be called from the main UI thread.
HRESULT AudioOutput::Init(
  AudioCallback callback) {
  OutputDebugString(L"AudioOutput::InitializeAudioDeviceAsync\n");

  m_audioCallback = callback;

  // Create events for sample ready or user stop
  m_SampleReadyEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
  if (nullptr == m_SampleReadyEvent) {
    //ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );
    return HRESULT_FROM_WIN32(GetLastError());
  }

  if (!InitializeCriticalSectionEx(&m_CritSec, 0, 0)) {
    //ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );
    return HRESULT_FROM_WIN32(GetLastError());
  }


  IActivateAudioInterfaceAsyncOperation *asyncOp;
  HRESULT hr = S_OK;

  // Get a string representing the Default Audio Device Renderer
  Platform::String^   m_DeviceIdString;

  m_DeviceIdString = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);

  // This call must be made on the main UI thread.  Async operation will call back to 
  // IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
  hr = ActivateAudioInterfaceAsync(m_DeviceIdString->Data(), __uuidof(IAudioClient2), nullptr, this, &asyncOp);
  if (FAILED(hr)) {
    m_deviceState = DeviceState::DeviceStateInError;
    //m_deviceState = DeviceState::DeviceStateInError;
  }

  SAFE_RELEASE(asyncOp);

  return hr;
}

//
//  ActivateCompleted()
//
//  Callback implementation of ActivateAudioInterfaceAsync function.  This will be called on MTA thread
//  when results of the activation are available.
//
HRESULT AudioOutput::ActivateCompleted(IActivateAudioInterfaceAsyncOperation *operation) {
  OutputDebugString(L"AudioOutput::ActivateCompleted\n");

  HRESULT hr = S_OK;
  HRESULT hrActivateResult = S_OK;
  IUnknown *punkAudioInterface = nullptr;

  //if (m_deviceState != DeviceState::DeviceStateUnInitialized)
  if (m_deviceState != DeviceState::DeviceStateUnInitialized) {
    hr = E_NOT_VALID_STATE;
    goto exit;
  }

  // Check for a successful activation result
  hr = operation->GetActivateResult(&hrActivateResult, &punkAudioInterface);
  if (SUCCEEDED(hr) && SUCCEEDED(hrActivateResult)) {
    //m_deviceState = DeviceState::DeviceStateActivated;
    m_deviceState = DeviceState::DeviceStateActivated;

    // Get the pointer for the Audio Client
    punkAudioInterface->QueryInterface(IID_PPV_ARGS(&m_AudioClient));
    if (nullptr == m_AudioClient) {
      hr = E_FAIL;
      goto exit;
    }

    // Configure user defined properties
    hr = ConfigureDeviceInternal();
    if (FAILED(hr)) {
      goto exit;
    }

    // Initialize the AudioClient in Shared Mode with the user specified buffer
    OutputDebugString(L"Calling m_AudioClient->Initialize\n");
    hr = m_AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,

      AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
      m_hnsBufferDuration,
      m_hnsBufferDuration,
      m_MixFormat,
      nullptr);
    if (FAILED(hr)) {
      goto exit;
    }
    OutputDebugString(L"After call to m_AudioClient->Initialize\n");

    // Get the maximum size of the AudioClient Buffer
    hr = m_AudioClient->GetBufferSize(&m_BufferFrames);
    if (FAILED(hr)) {
      goto exit;
    }

    // Make sure it's using 32-bit floating point, and not 16-bit shorts. 16-bit output
    // could be supported, but would require a bit of extra code in OnAudioSampleRequested to
    // convert the floats provided by the callback function.
    assert(CalculateMixFormatType(m_MixFormat) == SampleTypeFloat);

    //>>>>>>>>>>>>>>>>>
    DebugPrintf("m_MixFormat:\n");
    DebugPrintf("nAvgBytesPerSec = %d\n", m_MixFormat->nAvgBytesPerSec);
    DebugPrintf("nSamplesPerSec = %d\n", m_MixFormat->nSamplesPerSec);
    DebugPrintf("wBitsPerSample = %d\n", m_MixFormat->wBitsPerSample);
    DebugPrintf("nChannels = %d\n", m_MixFormat->nChannels);
    //<<<<<<<<<<<<<<<<<

    m_sampleRate = m_MixFormat->nSamplesPerSec;
    m_numChannels = m_MixFormat->nChannels;

    // Get the render client
    hr = m_AudioClient->GetService(__uuidof(IAudioRenderClient), (void**) &m_AudioRenderClient);
    if (FAILED(hr)) {
      goto exit;
    }

    // Create Async callback for sample events
    hr = MFCreateAsyncResult(nullptr, &m_xSampleReady, nullptr, &m_SampleReadyAsyncResult);
    if (FAILED(hr)) {
      goto exit;
    }

    // Sets the event handle that the system signals when an audio buffer is ready to be processed by the client
    hr = m_AudioClient->SetEventHandle(m_SampleReadyEvent);
    if (FAILED(hr)) {
      goto exit;
    }

    // Everything succeeded
    //m_deviceState = DeviceState::DeviceStateInitialized;
    m_deviceState = DeviceState::DeviceStateInitialized;

    // Automatically start the audio output.
    StartPlaybackAsync(); // Doesn't work if I call it here??
  }

exit:
  SAFE_RELEASE(punkAudioInterface);

  if (FAILED(hr)) {
    //m_deviceState = DeviceState::DeviceStateInError;
    m_deviceState = DeviceState::DeviceStateInError;
    SAFE_RELEASE(m_AudioClient);
    SAFE_RELEASE(m_AudioRenderClient);
    SAFE_RELEASE(m_SampleReadyAsyncResult);
  }

  // Need to return S_OK
  return S_OK;
}


//
//  ConfigureDeviceInternal()
//
//  Sets additional playback parameters and opts into hardware offload
//
HRESULT AudioOutput::ConfigureDeviceInternal() {
  assert(m_deviceState == DeviceStateActivated);

  HRESULT hr = S_OK;

  // Opt into HW Offloading.  If the endpoint does not support offload it will return AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE
  AudioClientProperties audioProps;
  memset(&audioProps, 0, sizeof(audioProps));
  audioProps.cbSize = sizeof(AudioClientProperties);
  audioProps.bIsOffload = m_IsHWOffload;
  audioProps.eCategory = (m_IsBackground ? AudioCategory_BackgroundCapableMedia : AudioCategory_ForegroundOnlyMedia);

  hr = m_AudioClient->SetClientProperties(&audioProps);
  if (FAILED(hr))
    return hr;

  // This sample opens the device is shared mode so we need to find the supported WAVEFORMATEX mix format
  hr = m_AudioClient->GetMixFormat(&m_MixFormat);
  if (FAILED(hr))
    return hr;

  // Verify the user defined value for hardware buffer
  hr = ValidateBufferValue();

  return hr;
}

//
//  ValidateBufferValue()
//
//  Verifies the user specified buffer value for hardware offload
//
HRESULT AudioOutput::ValidateBufferValue() {
  HRESULT hr = S_OK;

  //if (!m_DeviceProps.IsHWOffload)
  if (!m_IsHWOffload) {
    // If we aren't using HW Offload, set this to 0 to use the default value
    //m_DeviceProps.hnsBufferDuration = 0;
    m_hnsBufferDuration = 0;
    return hr;
  }

  REFERENCE_TIME hnsMinBufferDuration;
  REFERENCE_TIME hnsMaxBufferDuration;

  hr = m_AudioClient->GetBufferSizeLimits(m_MixFormat, true, &hnsMinBufferDuration, &hnsMaxBufferDuration);
  if (SUCCEEDED(hr)) {
    if (m_hnsBufferDuration < hnsMinBufferDuration) {
      // using MINIMUM size instead
      m_hnsBufferDuration = hnsMinBufferDuration;
    }
    else if (hnsMinBufferDuration > hnsMaxBufferDuration) {
      // using MAXIMUM size instead
      m_hnsBufferDuration = hnsMaxBufferDuration;
    }
  }

  return hr;
}


//
//  SetVolumeOnSession()
//
HRESULT AudioOutput::SetVolumeOnSession(float volume) {
  volume = min(volume, 1.0f);

  HRESULT hr = S_OK;
  ISimpleAudioVolume *SessionAudioVolume = nullptr;
  float ChannelVolume = 0.0;

  hr = m_AudioClient->GetService(__uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&SessionAudioVolume));

  if (SUCCEEDED(hr)) {
    // Set the session volume on the endpoint
    hr = SessionAudioVolume->SetMasterVolume(volume, nullptr);
  }

  SAFE_RELEASE(SessionAudioVolume);
  return hr;
}


//
//  StartPlaybackAsync()
//
//  Starts asynchronous playback on a separate thread via MF Work Item
//
HRESULT AudioOutput::StartPlaybackAsync() {
  OutputDebugString(L"StartPlaybackAsync\n");
  return MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_xStartPlayback, nullptr);
}

//
//  OnStartPlayback()
//
//  Callback method to start playback
//
HRESULT AudioOutput::OnStartPlayback(IMFAsyncResult* pResult) {
  OutputDebugString(L"OnStartPlayback\n");

  HRESULT hr = S_OK;

  try {
    // Pre-Roll the buffer with silence
    hr = OnAudioSampleRequested(true);
    ThrowIfFailed(hr);

    // Actually start the playback
    hr = m_AudioClient->Start();
    ThrowIfFailed(hr);

    m_deviceState = DeviceState::DeviceStatePlaying;
    hr = MFPutWaitingWorkItem(m_SampleReadyEvent, 0, m_SampleReadyAsyncResult, &m_SampleReadyKey);
    ThrowIfFailed(hr);
  }
  catch (Platform::Exception^ e) {
    m_deviceState = DeviceState::DeviceStateInError;
  }

  return S_OK;
}



//
//  OnSampleReady()
//
//  Callback method when ready to fill sample buffer
//
HRESULT AudioOutput::OnSampleReady(IMFAsyncResult* pResult) {
  HRESULT hr = S_OK;

  hr = OnAudioSampleRequested(false);

  if (SUCCEEDED(hr)) {
    // Re-queue work item for next sample
    if (m_deviceState == DeviceState::DeviceStatePlaying) {
      hr = MFPutWaitingWorkItem(m_SampleReadyEvent, 0, m_SampleReadyAsyncResult, &m_SampleReadyKey);
    }
  }
  else {
    m_deviceState = DeviceState::DeviceStateInError;
  }

  return hr;
}

//
//  OnAudioSampleRequested()
//
//  Called when audio device fires m_SampleReadyEvent
//
HRESULT AudioOutput::OnAudioSampleRequested(Platform::Boolean IsSilence) {
  HRESULT hr = S_OK;
  UINT32 paddingFrames = 0;
  UINT32 framesAvailable = 0;

  EnterCriticalSection(&m_CritSec);

  // Get padding in existing buffer
  hr = m_AudioClient->GetCurrentPadding(&paddingFrames);
  if (FAILED(hr)) {
    goto exit;
  }

  // Audio frames available in buffer
  if (m_IsHWOffload) {
    // In HW mode, GetCurrentPadding returns the number of available frames in the 
    // buffer, so we can just use that directly
    framesAvailable = paddingFrames;
  }
  else {
    // In non-HW shared mode, GetCurrentPadding represents the number of queued frames
    // so we can subtract that from the overall number of frames we have
    framesAvailable = m_BufferFrames - paddingFrames;
  }

  // With BootCamp / MacBook Pro, FrameAvailable is 441.
  //DebugPrintf("framesAvailable = %d\n", framesAvailable);


  // Only continue if we have buffer to write data
  if (framesAvailable > 0) {
    //		hr = GetToneSample( FramesAvailable );
    hr = S_OK;
    BYTE *Data;

    hr = m_AudioRenderClient->GetBuffer(framesAvailable, &Data);
    float32 *outBuf = (float32*) Data;

    if (!IsSilence) {
      // Call a client function to fill the buffer with audio
      m_audioCallback(outBuf, framesAvailable, m_numChannels, m_sampleRate);
    }
    else {
      memset(outBuf, 0, framesAvailable*m_numChannels*sizeof(outBuf[0]));
    }

    hr = m_AudioRenderClient->ReleaseBuffer(framesAvailable, 0);
  }

exit:
  LeaveCriticalSection(&m_CritSec);

  if (AUDCLNT_E_RESOURCES_INVALIDATED == hr) {
    m_deviceState = DeviceState::DeviceStateUnInitialized;
    SAFE_RELEASE(m_AudioClient);
    SAFE_RELEASE(m_AudioRenderClient);
    SAFE_RELEASE(m_SampleReadyAsyncResult);

    //        hr = InitializeAudioDeviceAsync();
  }

  return hr;
}

