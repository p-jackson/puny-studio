define(['knockback'], function(kb) {
  'use strict';

  return kb.ViewModel.extend({
    constructor: function() {
      kb.ViewModel.prototype.constructor.apply(this, arguments);
    }
  });
});