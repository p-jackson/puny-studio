#ifndef MIDI_PATCHES_H
#define MIDI_PATCHES_H

namespace midi {
  public ref class Patches sealed {
  public:
    Patches();

    property bool liveMode;
    property int activePatch;
  };
}

#endif