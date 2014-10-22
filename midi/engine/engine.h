#ifndef MIDI_ENGINE_H
#define MIDI_ENGINE_H

namespace midi {
  namespace engine {

    void start();
    void setInputChannel(int channelIndex);
    void addTone(double freq);
    void removeTone(double freq);

  }
}

#endif