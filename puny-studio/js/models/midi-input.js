define(['./storage-model', 'midi'], function(StorageModel, midi) {
  'use strict';

  return StorageModel.extend({
    storage: midi.MidiInput,

    defaults: {
      deviceName: 'Default Device',
      channel: 3,
      regions: 1,
      splitOne: Math.round(43 / 3),
      splitTwo: Math.round(43 * 2 / 3)
    }
  });
});