define(['backbone', 'knockout', 'knockback', 'bindings/number-chooser'], function(Backbone, ko, kb) {
  'use strict';

  function regionColor(regionIndex) {
    switch (regionIndex) {
      case 0: return 'blue-foreground';
      case 1: return 'green-foreground';
      default: return 'orange-foreground';
    }
  }

  function regionText(regionIndex) {
    switch (regionIndex) {
      case 0: return 'region one';
      case 1: return 'region two';
      default: return 'region three';
    }
  }

  function regionTextTitle(regionIndex) {
    switch (regionIndex) {
      case 0: return 'Region One';
      case 1: return 'Region Two';
      default: return 'Region Three';
    }
  }

  function RegionRowViewModel(rowIndex) {
    this.channel = ko.observable(rowIndex + 1);
    this.regionTextStyle = regionColor(rowIndex);
    this.regionText = regionText(rowIndex);
    this.regionTextTitle = regionTextTitle(rowIndex);
  }

  function BlackKey(index, parent) {
    this.background = ko.computed(function() {
      if (parent.regions() === 1)
        return 'highlight-foreground-as-background';
      else
        return 'highlight-foreground-as-background';
    });
  }

  function WhiteKey(index, parent) {
    this.background = ko.computed(function() {
      if (parent.regions() === 1)
        return 'blue-background-50';
      else if (parent.regions() === 2) {
        if (index < parent.splitOne())
          return 'blue-background-50';
        else
          return 'green-background-50';
      }
      else {
        if (index < parent.splitOne())
          return 'blue-background-50';
        else if (index < parent.splitTwo())
          return 'green-background-50';
        else
          return 'orange-background-50';
      }
    });
  }

  return kb.ViewModel.extend({
    constructor: function() {
      kb.ViewModel.prototype.constructor.apply(this, arguments);

      this.regionText = ko.computed(function() {
        return this.regions() < 2 ? 'region' : 'regions';
      }, this);

      var whiteKeys = [];
      for (var i = 0; i < 43; ++i)
        whiteKeys[i] = new WhiteKey(i, this);

      var blackKeys = [];
      for (i = 0; i < 30; ++i)
        blackKeys[i] = new BlackKey(i, this);

      this.whiteKeys = ko.observableArray(whiteKeys);
      this.blackKeys = ko.observableArray(blackKeys);

      this.regionRows = ko.observableArray();
      this.adjustRegionRows(this.regions());
      this.regions.subscribe(this.adjustRegionRows, this);
    },

    adjustRegionRows: function(numRegions) {
      var rows = [];
      for (var i = 0; i < numRegions; ++i)
        rows[i] = new RegionRowViewModel(i);

      this.regionRows(rows);
    }
  });
});