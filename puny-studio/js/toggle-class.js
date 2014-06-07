define(function() {
  'use strict';

  function toggleClass(el, classList, value) {
    var classes = classList.split(' ');
    for (var i = 0; i < classes.length; ++i) {
      if (value)
        el.classList.add(classes[i]);
      else
        el.classList.remove(classes[i]);
    }
  }

  return toggleClass;
});