#ifndef MIDI_MIDI_INPUT_H
#define MIDI_MIDI_INPUT_H

#include "settings.h"

namespace midi {
  public ref class MidiInput sealed {
    Settings m_settings;

  public:
    MidiInput();

    property Platform::String^ deviceName;
    property int channel;
    property int regions;
    property int splitOne;
    property int splitTwo;

    void afterUpdate();
  };
}

#endif