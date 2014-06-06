define(['knockout', 'jquery'], function(ko, $) {
  'use strict';

  ko.bindingHandlers['patchButton'] = {
    init: function(el, valueAccessor) {

      var patchNumber = parseInt($(el).attr('data-patch'), 10);

      function updateButton() {
        var isActive = ko.unwrap(valueAccessor()) === patchNumber;

        $(el).toggleClass('isActive', isActive);
        $(el).toggleClass('cyan-background on-colour-foreground highlight-foreground-border', isActive);
        $(el).toggleClass('highlight-background highlight-foreground primary-foreground-as-border-10', !isActive);
      }

      updateButton();

      $(el).mousedown(function() {
        var observable = valueAccessor();
        observable(patchNumber);
      });

      valueAccessor().subscribe(function() {
        updateButton();
      });
    }
  };
});