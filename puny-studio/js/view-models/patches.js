define(['knockout', 'knockback', 'bindings/on-off', 'bindings/patch-button'], function(ko, kb) {
  'use strict';

  return kb.ViewModel.extend({
    constructor: function() {
      kb.ViewModel.prototype.constructor.apply(this, arguments);

      this.rememberedText = ko.computed(function() {
        if (this.liveMode())
          return 'Your changes to patches 1&ndash;4 will be remembered'
        else
          return 'Settings will be reset to their initial values as you switch between patches';
      }, this);
    }
  });
});