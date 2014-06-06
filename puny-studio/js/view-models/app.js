define(['knockout', 'knockback', 'bindings/history-link', 'bindings/toggle-theme'], function(ko, kb) {
  'use strict';

  return kb.ViewModel.extend({
    constructor: function() {
      kb.ViewModel.prototype.constructor.apply(this, arguments);
      
      this.mainTemplate = ko.observable('empty');
      this.mainViewModel = ko.observable();
      this.mainView = ko.observable({
        name: 'empty',
        data: undefined
      });
    },

    setMainView: function(name, vm) {
      this.mainView({
        name: name,
        data: vm
      });
      this.mainTemplate(name);
      this.mainViewModel(vm);
    }
  });
});