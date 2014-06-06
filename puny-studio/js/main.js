requirejs.config({
  baseUrl: 'js',

  paths: {
    'backbone': '../lib/backbone',
    'underscore': '../lib/underscore',
    'jquery': '../lib/zepto',
    'knockout': '../lib/knockout',
    'knockback': '../lib/knockback'
  },

  shim: {
    backbone: {
      deps: ['underscore', 'jquery']
    },
    knockback: {
      deps: ['knockout', 'backbone']
    },
    jquery: {
      exports: '$'
    }
  }
});

define([
  'knockout',
  'backbone',
  'models/app',
  'view-models/app',
  'router',
  'init-winjs'],
function(ko, Backbone, App, AppVM, router, initWinJs) {
  "use strict";

  initWinJs();

  var app = new App();
  var vm = new AppVM(app);

  router.setAppViewModel(vm);

  ko.applyBindings(vm);

  Backbone.history.start({ pushState: false });
  
});