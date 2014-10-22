define(['backbone', 'underscore'], function(Backbone, _) {
  'use strict';

  function functionName(fun) {
    var ret = fun.toString();
    ret = ret.substr('function '.length);
    ret = ret.substr(0, ret.indexOf('('));
    return ret.trim();
  }

  var storageEvents = _.extend({}, Backbone.Events);

  var StorageModel = Backbone.Model.extend({
    constructor: function() {
      Backbone.Model.prototype.constructor.apply(this, arguments);
      if (!this.storage)
        return;

      this._storage = new this.storage();
      this.on('change', this.save, this);
      this.listenTo(storageEvents, 'suspend', this.save, this);
      this.listenTo(storageEvents, 'restore', this.fetch, this);

      // When save is called, it should cause an update, not a create.
      this.id = true;
    },

    sync: function(method, model) {
      if (!model._storage)
        // No storage, not in a Metro environment
        return;

      var beforeMethod = 'before' + method[0].toUpperCase() + method.substr(1);
      if (model._storage[beforeMethod])
        model._storage[beforeMethod]();

      var keys = _.keys(_.result(model, 'defaults'));
      keys.forEach(function(key) {
        if (_.isUndefined(model._storage[key]))
          throw new Error('Missing property on ' + functionName(model.storage) + ' storage type: ' + key);

        switch (method) {
          case 'read':
            model.set(key, model._storage[key], { silent: true });
            break;
          case 'update':
            model._storage[key] = model.get(key);
            break;
        }
        
      });

      var afterMethod = 'after' + method[0].toUpperCase() + method.substr(1);
      if (model._storage[afterMethod])
        model._storage[afterMethod]();
    }
  });

  _.extend(StorageModel, {
    suspend: function() {
      storageEvents.trigger('suspend');
    },

    restore: function() {
      storageEvents.trigger('restore');
    }
  });

  return StorageModel;

});