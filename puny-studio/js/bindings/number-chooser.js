define(['underscore', 'knockout', 'create-element'], function(_, ko, createElement) {
  'use strict';

  ko.bindingHandlers['numberChooser'] = {
    init: function(el, valueAccessor) {

      var initialValue = ko.unwrap(valueAccessor().number);
      var min = valueAccessor().min;
      var max = valueAccessor().max;

      var num = createElement('span')
                  .addClass('numberChooser-number highlight-background highlight-foreground')
                  .text(initialValue)
                  .appendTo(el)
                  .get();

      var inc = createElement('button')
                  .addClass('numberChooser-increase blue-background')
                  .appendTo(el)
                  .get();

      var dec = createElement('button')
                  .addClass('numberChooser-decrease blue-background')
                  .appendTo(el)
                  .get();

      var done = createElement('button')
                  .addClass('numberChooser-done green-background')
                  .appendTo(el)
                  .get();

      var punyStudioNumberChooserId = _.uniqueId();

      function windowMouseDown(e) {
        var id = e._punyStudioNumberChooser;
        if (id !== punyStudioNumberChooserId) {
          el.classList.remove('isActive');
          window.removeEventListener('mousedown', windowMouseDown);
        }
      }

      el.addEventListener('mousedown', function(e) {
        e._punyStudioNumberChooser = punyStudioNumberChooserId;
      }, true);

      ko.utils.domNodeDisposal.addDisposeCallback(el, function() {
        window.removeEventListener('mousedown', windowMouseDown);
      });

      num.addEventListener('mousedown', function() {
        el.classList.add('isActive');
        window.addEventListener('mousedown', windowMouseDown);
      });

      function updateEnableStates() {
        var number = ko.unwrap(valueAccessor().number);
        inc.disabled = number >= max;
        dec.disabled = number <= min;
      }

      updateEnableStates();

      inc.addEventListener('mousedown', function(e) {
        var number = valueAccessor().number;
        if (number() < max)
          number(number() + 1);
        updateEnableStates();
      });

      dec.addEventListener('mousedown', function(e) {
        var number = valueAccessor().number;
        if (number() > min)
          number(number() - 1);
        updateEnableStates();
      });

      done.addEventListener('mousedown', function(e) {
        el.classList.remove('isActive');
      });

      valueAccessor().number.subscribe(function(newValue) {
        num.textContent = newValue;
      });
    }
  };
});