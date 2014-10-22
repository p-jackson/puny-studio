define(function() {
  'use strict';

  if (!window.midi || !midi.MidiInput)
    // Not in the Metro environment
    return {};

  return {
    MidiInput: midi.MidiInput,
    Patches: midi.Patches,
    EngineControl: midi.EngineControl
  };

});