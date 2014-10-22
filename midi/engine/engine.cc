#include "engine.h"

#include "audio-output.h"

#include <cassert>
#include <objbase.h>

using namespace WindowsPreview::Devices::Midi;
using namespace Concurrency;
using namespace Windows::Foundation;

static const double k_pi = 4 * atan(1.0);
static const double k_2pi = k_pi;

std::atomic<int> g_channelIndex;
std::atomic<bool> g_tone = true;

AudioOutput g_audio;
double g_phase = 0.0;
double g_frequency = 440.0;

void generateSilence(float32 *output, int n, int numChannels, int sampleRate) {
  for (int i = 0; i < n; i++) {
    for (int ch = 0; ch < numChannels; ch++)
      *output++ = 0;
  }
}

void addSine(float32 *output, int n, int numChannels, int sampleRate, float amp) {
  assert(g_frequency != 0);
  double phaseInc = k_2pi * g_frequency / sampleRate;

  // Generate the samples
  for (int i = 0; i < n; i++) {
    float32 x = float(amp * sin(g_phase));
    for (int ch = 0; ch < numChannels; ch++)
      *output++ += x;
    g_phase += phaseInc;
  }

  // Bring phase back into range [0, 2pi]
  g_phase = fmod(g_phase, k_2pi);
}

void addTriangle(float32 *output, int n, int numChannels, int sampleRate, float amp) {
  assert(g_frequency != 0);
  const auto phaseInc = k_2pi * g_frequency / sampleRate;

  // Generate the samples
  for (int i = 0; i < n; i++) {
    float32 x = 0;
    if (g_phase <= k_pi)
      x = static_cast<float>(2 * amp * g_phase / k_pi - amp);
    else
      x = static_cast<float>(-2 * amp * (g_phase - k_pi) / k_pi + amp);
    for (int ch = 0; ch < numChannels; ch++)
      *output++ += x;
    g_phase += phaseInc;
  }

  // Bring phase back into range [0, 2pi]
  g_phase = fmod(g_phase, k_2pi);
}

void addSaw(float32 *output, int n, int numChannels, int sampleRate, float amp) {
  assert(g_frequency != 0);
  double phaseInc = k_2pi * g_frequency / sampleRate;

  // Generate the samples
  for (int i = 0; i < n; i++) {
    float32 x = static_cast<float>(-2 * amp * g_phase / (2*k_pi) + amp);
    for (int ch = 0; ch < numChannels; ch++)
      *output++ += x;
    g_phase += phaseInc;
  }

  // Bring phase back into range [0, 2pi]
  g_phase = fmod(g_phase, k_2pi);
}

void addSquare(float32 *output, int n, int numChannels, int sampleRate, float amp) {
  assert(g_frequency != 0);
  double phaseInc = k_2pi * g_frequency / sampleRate;

  // Generate the samples
  for (int i = 0; i < n; i++) {
    float32 x = g_phase <= k_pi ? amp : (-1 * amp);
    for (int ch = 0; ch < numChannels; ch++)
      *output++ += x;
    g_phase += phaseInc;
  }

  // Bring phase back into range [0, 2pi]
  g_phase = fmod(g_phase, k_2pi);
}

void generate(float32 *output, int n, int numChannels, int sampleRate) {
  generateSilence(output, n, numChannels, sampleRate);
  if (g_tone)
    addSine(output, n, numChannels, sampleRate, 0.1f);
}

MidiInPort^ g_input = nullptr;

static void onMessageRecieved(MidiInPort^, MidiMessageReceivedEventArgs^) {

}

static void initMidi() {
  using namespace Windows::Devices::Enumeration;
  
  auto selector = MidiInPort::GetDeviceSelector();
  auto task = create_task(DeviceInformation::FindAllAsync(selector));

  task.then([](DeviceInformationCollection^ devices) {
    auto info = devices->GetAt(0);
    create_task(MidiInPort::FromIdAsync(info->Id)).then([](MidiInPort^ port) {
      g_input = port;
      g_input->MessageReceived += ref new TypedEventHandler<MidiInPort^, MidiMessageReceivedEventArgs^>(onMessageRecieved);
    });
  });
  
}

void midi::engine::start() {
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  MFStartup(MF_VERSION, MFSTARTUP_LITE);
  g_audio.Init(generate);
  initMidi();
}

void midi::engine::setInputChannel(int channelIndex) {
  g_channelIndex = channelIndex;
}

void midi::engine::addTone(double freq) {
  g_frequency = freq;
  g_tone = true;
}

void midi::engine::removeTone(double freq) {
  g_frequency = false;
}
