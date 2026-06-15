(function() {
  if (window.__rtranslator_state) return;
  window.__rtranslator_state = {};

  function resolvePath(path) {
    var parts = path.split('.');
    var segment = parts[0];
    if (segment === 'party') return resolvePartyPath(parts.slice(1));
    if (segment === 'actors') return resolveActorPath(parts.slice(1));
    if (segment === 'switches') return resolveSwitchPath(parts.slice(1));
    if (segment === 'variables') return resolveVariablePath(parts.slice(1));
    if (segment === 'items') return resolveItemPath(parts.slice(1));
    if (segment === 'weapons') return resolveWeaponPath(parts.slice(1));
    if (segment === 'armors') return resolveArmorPath(parts.slice(1));
    if (segment === 'troop') return resolveTroopPath(parts.slice(1));
    return null;
  }

  function resolvePartyPath(parts) {
    if (!window.$gameParty) return null;
    return {
      get: function() {
        if (parts[0] === 'gold') return $gameParty.gold();
        if (parts[0] === 'steps') return $gameParty.steps();
        return null;
      },
      set: function(val) {
        if (parts[0] === 'gold') { $gameParty._gold = val; return true; }
        return false;
      }
    };
  }

  function resolveActorPath(parts) {
    if (!window.$gameActors || parts.length < 2) return null;
    var actorId = parseInt(parts[0], 10);
    var prop = parts[1];
    return {
      get: function() {
        var actor = $gameActors.actor(actorId);
        if (!actor) return null;
        if (prop === 'hp') return actor.hp;
        if (prop === 'mp') return actor.mp;
        if (prop === 'level') return actor._level;
        if (prop === 'exp') return actor.currentExp ? actor.currentExp() : 0;
        if (prop === 'name') return actor.name();
        if (prop === 'atk') return actor.atk;
        if (prop === 'def') return actor.def;
        if (prop === 'mat') return actor.mat;
        if (prop === 'mdf') return actor.mdf;
        if (prop === 'agi') return actor.agi;
        if (prop === 'luk') return actor.luk;
        return null;
      },
        set: function(val) {
          var actor = $gameActors.actor(actorId);
          if (!actor) return false;
          if (prop === 'hp') { actor._hp = Math.max(0, Math.min(val, actor.mhp)); return true; }
          if (prop === 'mp') { actor._mp = Math.max(0, Math.min(val, actor.mmp)); return true; }
          if (prop === 'level') { actor._level = val; return true; }
          if (prop === 'exp') { actor._exp = {1: val}; return true; }
          return false;
        }
    };
  }

  function resolveSwitchPath(parts) {
    if (!window.$gameSwitches) return null;
    var id = parseInt(parts[0], 10);
    return {
      get: function() { return $gameSwitches.value(id); },
      set: function(val) { $gameSwitches._data[id] = !!val; return true; }
    };
  }

  function resolveVariablePath(parts) {
    if (!window.$gameVariables) return null;
    var id = parseInt(parts[0], 10);
    return {
      get: function() { return $gameVariables.value(id); },
      set: function(val) { $gameVariables._data[id] = val; return true; }
    };
  }

  function resolveItemPath(parts) {
    if (!window.$gameParty || !window.$dataItems) return null;
    var id = parseInt(parts[0], 10);
    return {
      get: function() { return $gameParty.numItems($dataItems[id]); },
      set: function(val) {
        var cur = $gameParty.numItems($dataItems[id]);
        var diff = val - cur;
        if (diff > 0) $gameParty.gainItem($dataItems[id], diff);
        else if (diff < 0) $gameParty.loseItem($dataItems[id], -diff);
        return true;
      }
    };
  }

  function resolveWeaponPath(parts) {
    if (!window.$gameParty || !window.$dataWeapons) return null;
    var id = parseInt(parts[0], 10);
    return {
      get: function() { return $gameParty.numItems($dataWeapons[id]); },
      set: function(val) {
        var cur = $gameParty.numItems($dataWeapons[id]);
        var diff = val - cur;
        if (diff > 0) $gameParty.gainItem($dataWeapons[id], diff);
        else if (diff < 0) $gameParty.loseItem($dataWeapons[id], -diff);
        return true;
      }
    };
  }

  function resolveArmorPath(parts) {
    if (!window.$gameParty || !window.$dataArmors) return null;
    var id = parseInt(parts[0], 10);
    return {
      get: function() { return $gameParty.numItems($dataArmors[id]); },
      set: function(val) {
        var cur = $gameParty.numItems($dataArmors[id]);
        var diff = val - cur;
        if (diff > 0) $gameParty.gainItem($dataArmors[id], diff);
        else if (diff < 0) $gameParty.loseItem($dataArmors[id], -diff);
        return true;
      }
    };
  }

  function resolveTroopPath(parts) {
    if (!window.$gameTroop) return null;
    return {
      get: function() {
        if (parts[0] === 'alive') return $gameTroop.aliveMembers().length;
        if (parts[0] === 'dead') return $gameTroop.deadMembers().length;
        if (parts[0] === 'turn') return $gameTroop.turnCount();
        if (parts[0] === 'id') return $gameTroop._troopId;
        return null;
      },
      set: function(val) { return false; }
    };
  }

  window.__rtranslator_readState = function(path) {
    var acc = resolvePath(path);
    return acc ? acc.get() : null;
  };
  window.__rtranslator_writeState = function(path, value) {
    var acc = resolvePath(path);
    return acc ? acc.set(value) : false;
  };
})();
