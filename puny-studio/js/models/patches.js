define(['./storage-model', 'midi'], function(StorageModel, midi) {
  'use strict';

  return StorageModel.extend({
    storage: midi.Patches,

    defaults: {
      liveMode: false,
      activePatch: 1
    }
  });
});