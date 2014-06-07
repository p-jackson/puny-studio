define(function() {
  'use strict';

  function unwrap(a) {
    return a instanceof ElementWrapper ? a.el : a;
  }

  function ElementWrapper(el) {
    this.el = el;
  }

  _.extend(ElementWrapper.prototype, {
    get: function() {
      return this.el;
    },

    addClass: function(classNames) {
      var classes = classNames.split(' ');
      for (var i = 0; i < classes.length; ++i)
        this.el.classList.add(classes[i]);

      return this;
    },

    text: function(text) {
      this.el.textContent = text;
      return this;
    },

    appendTo: function(newParent) {
      unwrap(newParent).appendChild(this.el);
      return this;
    }
  });

  function createElement(tagName) {
    return new ElementWrapper(document.createElement(tagName));
  }

  return createElement;
});