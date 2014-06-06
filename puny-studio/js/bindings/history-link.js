define(['knockout', 'router'], function(ko, router) {
  'use strict';

  ko.bindingHandlers['historyLink'] = {
    init: function(el, valueAccessor) {
      var href = el.getAttribute('href');

      el.addEventListener('mousedown', function(e) {
        router.navigate(href);
      });

      el.addEventListener('click', function(e) {
        e.preventDefault();
      });

      router.on('route', function() {
        var parts;
        var hash = window.location.hash;
        if (!hash.length)
          parts = ['']
        else
          parts = hash.substr(1).split('/');

        var historyDepth = ko.unwrap(valueAccessor());
        if (historyDepth < parts.length && parts[historyDepth] === href) {
          el.classList.add('isCurrent');
          el.classList.add('cyan-background');
        }
        else {
          el.classList.remove('isCurrent');
          el.classList.remove('cyan-background');
        }
      });
    }
  };
});