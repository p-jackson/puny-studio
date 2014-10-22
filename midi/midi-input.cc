#include "midi-input.h"

#include "engine/engine.h"

using namespace midi;
using namespace Platform;

MidiInput::MidiInput()
  : m_settings{ getSettings(L"midi_input") }
{
  deviceName = L"Massive Device Name";
  channel = m_settings.get<int>(L"channel", 13);
  regions = 1;
  splitOne = 43 / 3;
  splitTwo = 43 * 2 / 3;
}

void MidiInput::afterUpdate() {
  m_settings.set<int>(L"channel", channel);

  engine::setInputChannel(channel - 1);

  m_settings.save();
}
