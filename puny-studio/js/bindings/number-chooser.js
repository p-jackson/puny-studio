define(['knockout', 'jquery'], function(ko, $) {
  'use strict';

  ko.bindingHandlers['numberChooser'] = {
    init: function(el, valueAccessor) {

      var initialValue = ko.unwrap(valueAccessor().number);
      var min = valueAccessor().min;
      var max = valueAccessor().max;

      var num = $('<span>').addClass('numberChooser-number highlight-background highlight-foreground').text(initialValue);
      var inc = $('<button>').addClass('numberChooser-increase blue-background');
      var dec = $('<button>').addClass('numberChooser-decrease blue-background');
      var done = $('<button>').addClass('numberChooser-done green-background');

      num.appendTo(el);
      inc.appendTo(el);
      dec.appendTo(el);
      done.appendTo(el);

      var punyStudioNumberChooserId = _.uniqueId();

      function windowMouseDown(e) {
        var id = e._punyStudioNumberChooser;
        if (id !== punyStudioNumberChooserId) {
          $(el).removeClass('isActive');
          window.removeEventListener('mousedown', windowMouseDown);
        }
      }

      el.addEventListener('mousedown', function(e) {
        e._punyStudioNumberChooser = punyStudioNumberChooserId;
      }, true);

      ko.utils.domNodeDisposal.addDisposeCallback(el, function() {
        window.removeEventListener('mousedown', windowMouseDown);
      });

      num.mousedown(function() {
        $(el).addClass('isActive');
        window.addEventListener('mousedown', windowMouseDown);
      });

      function updateEnableStates() {
        var number = ko.unwrap(valueAccessor().number);
        inc.prop('disabled', number >= max);
        dec.prop('disabled', number <= min);
      }

      updateEnableStates();

      inc.mousedown(function(e) {
        var number = valueAccessor().number;
        if (number() < max)
          number(number() + 1);
        updateEnableStates();
      });

      dec.mousedown(function(e) {
        var number = valueAccessor().number;
        if (number() > min)
          number(number() - 1);
        updateEnableStates();
      });

      done.mousedown(function(e) {
        $(el).removeClass('isActive');
      });

      valueAccessor().number.subscribe(function(newValue) {
        num.text(newValue);
      });
    }
  };
});