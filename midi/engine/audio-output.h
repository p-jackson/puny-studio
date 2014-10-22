//
// AudioOutput.cpp
//
// WASAPI-based audio output class for real-time audio generation.
// Author: Gerry Beauregard, 2013
// 
#pragma once

#include <Windows.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>
#include "Common.h"

using namespace Windows::Media::Devices;

typedef void(*AudioCallback)(float32 *output, int n, int numChannels, int sampleRate);

// AudioOutput
class AudioOutput :
  public RuntimeClass< RuntimeClassFlags< ClassicCom >, FtmBase, IActivateAudioInterfaceCompletionHandler > {
public:
  AudioOutput();
  ~AudioOutput();

  HRESULT Init(AudioCallback callback);
  HRESULT SetVolumeOnSession(float volume);

private:
  HRESULT StartPlaybackAsync();

  METHODASYNCCALLBACK(AudioOutput, StartPlayback, OnStartPlayback);
  METHODASYNCCALLBACK(AudioOutput, SampleReady, OnSampleReady);

  // IActivateAudioInterfaceCompletionHandler
  STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation);

  HRESULT OnStartPlayback(IMFAsyncResult* pResult);
  HRESULT OnSampleReady(IMFAsyncResult* pResult);

  HRESULT ConfigureDeviceInternal();
  HRESULT ValidateBufferValue();
  HRESULT OnAudioSampleRequested(Platform::Boolean IsSilence = false);

  typedef enum {
    DeviceStateUnInitialized,
    DeviceStateInError,
    DeviceStateActivated,
    DeviceStateInitialized,
    DeviceStateStarting,
    DeviceStatePlaying,
  } DeviceState;

  AudioCallback		m_audioCallback;
  int					m_numChannels;
  int					m_sampleRate;

  UINT32              m_BufferFrames;
  HANDLE              m_SampleReadyEvent;
  MFWORKITEM_KEY      m_SampleReadyKey;
  CRITICAL_SECTION    m_CritSec;

  WAVEFORMATEX           *m_MixFormat;
  IAudioClient2          *m_AudioClient;
  IAudioRenderClient     *m_AudioRenderClient;
  IMFAsyncResult         *m_SampleReadyAsyncResult;

  //RenderSampleType	m_renderSampleType;
  DeviceState m_deviceState;

  Platform::Boolean       m_IsHWOffload;
  Platform::Boolean       m_IsBackground;
  REFERENCE_TIME          m_hnsBufferDuration;
};

