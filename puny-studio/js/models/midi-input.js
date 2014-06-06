define(['backbone'], function(Backbone) {
  'use strict';

  return Backbone.Model.extend({
    defaults: {
      channel: 3,
      regions: 1,
      splitOne: Math.round(43 / 3),
      splitTwo: Math.round(43 * 2 / 3)
    }
  });
});