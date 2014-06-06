define(['knockout', 'jquery'], function(ko, $) {
  'use strict';

  var lightHref = 'css/light-colours.css';
  var darkHref = 'css/dark-colours.css';

  var transitionTime = 500;

  function addTransitionRules() {
    if ($('style#toggleThemeTransition').length)
      return;

    var styleRules =
      '*, *::after, *::before {' +
      '  transition-duration: ' + transitionTime + 'ms;' +
      '  transition-property: background-color border-color color;' +
      '}';

    var el = $('<style id="toggleThemeTransition">');
    el.html(styleRules)
    el.appendTo('head');
  }

  function removeTransitionRules() {
    $('style#toggleThemeTransition').remove();
  }

  ko.bindingHandlers['toggleTheme'] = {
    init: function(el, valueAccessor) {

      function onTransitionEnd() {
        removeTransitionRules();
        document.body.removeEventListener('transitionend', onTransitionEnd);
      }

      el.addEventListener('mousedown', function(e) {
        addTransitionRules();

        document.body.addEventListener('transitionend', onTransitionEnd);

        var style = document.querySelector('link#theme');
        var href = style.getAttribute('href');
        if (href === lightHref)
          style.setAttribute('href', darkHref);
        else if (href === darkHref)
          style.setAttribute('href', lightHref);
      });

      ko.utils.domNodeDisposal.addDisposeCallback(el, function() {
        removeTransitionRules();
        document.body.removeEventListener('transitionend', onTransitionEnd);
      });

      el.addEventListener('click', function(e) {
        e.preventDefault();
      });
    }
  };
});