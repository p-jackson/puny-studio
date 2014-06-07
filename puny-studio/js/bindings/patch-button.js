define(['knockout', 'toggle-class'], function(ko, toggleClass) {
  'use strict';

  ko.bindingHandlers['patchButton'] = {
    init: function(el, valueAccessor) {

      var patchNumber = parseInt(el.getAttribute('data-patch'), 10);

      function updateButton() {
        var isActive = ko.unwrap(valueAccessor()) === patchNumber;

        toggleClass(el, 'isActive', isActive);
        toggleClass(el, 'cyan-background on-colour-foreground highlight-foreground-border', isActive);
        toggleClass(el, 'highlight-background highlight-foreground primary-foreground-as-border-10', !isActive);
      }

      updateButton();

      el.addEventListener('mousedown', function() {
        var observable = valueAccessor();
        observable(patchNumber);
      });

      valueAccessor().subscribe(function() {
        updateButton();
      });
    }
  };
});