define(['underscore', 'knockout', 'jquery'], function(_, ko, $) {
  'use strict';

  function toText(value) {
    return value ? 'On' : 'Off';
  }

  ko.bindingHandlers['onOff'] = {
    init: function(el, valueAccessor) {

      var initialValue = ko.unwrap(valueAccessor());

      var val = $('<span>').addClass('onOff-value');
      var toggle = $('<span>').addClass('onOff-toggle on-colour-foreground');

      val.text(toText(initialValue)).appendTo(el);
      toggle.text(toText(!initialValue)).appendTo(el);

      function updateText() {
        var value = ko.unwrap(valueAccessor());
        val.text(toText(value));
        toggle.text(toText(!value));
        val.toggleClass('cyan-background on-colour-foreground', value);
        val.toggleClass('highlight-background highlight-foreground', !value);
        toggle.toggleClass('blue-background', !value);
        toggle.toggleClass('red-background', value);
      }

      updateText();

      var punyStudioOnOffId = _.uniqueId();

      function windowMouseDown(e) {
        var id = e._punyStudioOnOff;
        if (id !== punyStudioOnOffId) {
          $(el).removeClass('isActive');
          window.removeEventListener('mousedown', windowMouseDown);
        }
      }

      el.addEventListener('mousedown', function(e) {
        e._punyStudioOnOff = punyStudioOnOffId;
      }, true);

      val.mousedown(function() {
        $(el).addClass('isActive');
        window.addEventListener('mousedown', windowMouseDown);
      });

      ko.utils.domNodeDisposal.addDisposeCallback(el, function() {
        window.removeEventListener('mousedown', windowMouseDown);
      });

      toggle.mousedown(function(e) {
        var observable = valueAccessor();
        observable(!observable());
        $(el).removeClass('isActive');
      });

      valueAccessor().subscribe(function(newValue) {
        updateText();
      });
    }
  };
});