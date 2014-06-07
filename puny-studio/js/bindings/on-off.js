define(['underscore', 'knockout', 'create-element', 'toggle-class'], function(_, ko, createElement, toggleClass) {
  'use strict';

  function toText(value) {
    return value ? 'On' : 'Off';
  }

  ko.bindingHandlers['onOff'] = {
    init: function(el, valueAccessor) {

      var initialValue = ko.unwrap(valueAccessor());

      var val = createElement('span')
                  .addClass('onOff-value')
                  .text(toText(initialValue))
                  .appendTo(el)
                  .get();

      var toggle = createElement('span')
                    .addClass('onOff-toggle on-colour-foreground')
                    .text(toText(!initialValue))
                    .appendTo(el)
                    .get();

      function updateText() {
        var value = ko.unwrap(valueAccessor());
        val.textContent = toText(value);
        toggle.textContent = toText(!value);
        toggleClass(val, 'cyan-background on-colour-foreground', value);
        toggleClass(val, 'highlight-background highlight-foreground', !value);
        toggleClass(toggle, 'blue-background', !value);
        toggleClass(toggle, 'red-background', value);
      }

      updateText();

      var punyStudioOnOffId = _.uniqueId();

      function windowMouseDown(e) {
        var id = e._punyStudioOnOff;
        if (id !== punyStudioOnOffId) {
          el.classList.remove('isActive');
          window.removeEventListener('mousedown', windowMouseDown);
        }
      }

      el.addEventListener('mousedown', function(e) {
        e._punyStudioOnOff = punyStudioOnOffId;
      }, true);

      val.addEventListener('mousedown', function() {
        el.classList.add('isActive');
        window.addEventListener('mousedown', windowMouseDown);
      });

      ko.utils.domNodeDisposal.addDisposeCallback(el, function() {
        window.removeEventListener('mousedown', windowMouseDown);
      });

      toggle.addEventListener('mousedown', function(e) {
        var observable = valueAccessor();
        observable(!observable());
        el.classList.remove('isActive');
      });

      valueAccessor().subscribe(function(newValue) {
        updateText();
      });
    }
  };
});