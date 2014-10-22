define(['underscore', 'backbone'], function(_, Backbone) {
  'use strict';

  var Router = Backbone.Router.extend({
    setAppViewModel: function(vm) {
      this.vm = vm;
    },

    navigate: function(fragment, options) {
      options = _.defaults(options || {}, { trigger: true });
      Backbone.Router.prototype.navigate.call(this, fragment, options);
    },

    routes: {
      '': 'empty',
      'midi-input': 'midiInput',
      'about': 'about',
      'patches': 'patches'
    },

    setMainViewModel: function(viewName) {
      var parentViewModel = this.vm;
      var vmModule = './view-models/' + viewName;
      var modelModule = './models/' + viewName;

      require([vmModule, modelModule], function(ViewModel, Model) {
        var model = new Model();
        model.fetch();
        var vm = new ViewModel(model);
        parentViewModel.setMainView(viewName, vm);
      });
    },

    empty: function() {
      this.vm.setMainView('empty', undefined);
    },

    midiInput: function() {
      this.setMainViewModel('midi-input');
    },

    about: function() {
      this.setMainViewModel('about');
    },

    patches: function() {
      this.setMainViewModel('patches');
    }
  });

  return new Router();
});