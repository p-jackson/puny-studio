define(['underscore', 'models/storage-model', 'midi'], function(_, StorageModel, midi) {
  'use strict';

  return function() {
    if (_.isUndefined(window.WinJS))
      // Not in a WinJS environment
      return;

    var app = WinJS.Application;
    var activation = Windows.ApplicationModel.Activation;

    var engineControl = new midi.EngineControl();
    engineControl.start();

    app.onactivated = function(args) {
      if (args.detail.kind === activation.ActivationKind.launch) {
        if (args.detail.previousExecutionState !== activation.ApplicationExecutionState.terminated) {
          // TODO: This application has been newly launched. Initialize
          // your application here.
        } else {
          // TODO: This application has been reactivated from suspension.
          // Restore application state here.
        }

        StorageModel.restore();
        engineControl.start();

        args.setPromise(WinJS.UI.processAll());
      }
    };

    app.oncheckpoint = function(args) {
      // TODO: This application is about to be suspended. Save any state
      // that needs to persist across suspensions here. You might use the
      // WinJS.Application.sessionState object, which is automatically
      // saved and restored across suspension. If you need to complete an
      // asynchronous operation before your application is suspended, call
      // args.setPromise().

      StorageModel.suspend();
    };

    app.start();
  }
});