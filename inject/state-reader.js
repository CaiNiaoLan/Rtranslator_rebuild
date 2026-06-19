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
        if (parts[0] === 'size') return $gameParty.members().length;
        return null;
      },
      set: function(val) {
        if (parts[0] === 'gold') { $gameParty._gold = Math.max(0, val); return true; }
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
        if (prop === 'mhp') return actor.mhp;
        if (prop === 'mmp') return actor.mmp;
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
        if (prop === 'level') { actor._level = val; actor.refresh(); return true; }
        if (prop === 'atk') { actor._paramPlus[2] = (val - actor.paramBase(2)); actor.refresh(); return true; }
        if (prop === 'def') { actor._paramPlus[3] = (val - actor.paramBase(3)); actor.refresh(); return true; }
        if (prop === 'mat') { actor._paramPlus[4] = (val - actor.paramBase(4)); actor.refresh(); return true; }
        if (prop === 'mdf') { actor._paramPlus[5] = (val - actor.paramBase(5)); actor.refresh(); return true; }
        if (prop === 'agi') { actor._paramPlus[6] = (val - actor.paramBase(6)); actor.refresh(); return true; }
        if (prop === 'luk') { actor._paramPlus[7] = (val - actor.paramBase(7)); actor.refresh(); return true; }
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

  // === CHEAT FUNCTIONS ===

  window.__rtranslator_readState = function(path) {
    var acc = resolvePath(path);
    return acc ? acc.get() : null;
  };
  window.__rtranslator_writeState = function(path, value) {
    var acc = resolvePath(path);
    return acc ? acc.set(value) : false;
  };

  // Recover all party members
  window.__rtranslator_cheat_recoverAll = function() {
    if (!window.$gameParty) return false;
    var members = $gameParty.members();
    for (var i = 0; i < members.length; i++) {
      members[i].recoverAll();
    }
    return true;
  };

  // Max gold
  window.__rtranslator_cheat_maxGold = function() {
    if (!window.$gameParty) return false;
    $gameParty._gold = 99999999;
    return true;
  };

  // Give all items/weapons/armors
  window.__rtranslator_cheat_allItems = function() {
    if (!window.$gameParty) return false;
    if (window.$dataItems) for (var i = 1; i < $dataItems.length; i++) {
      if ($dataItems[i]) $gameParty.gainItem($dataItems[i], 99);
    }
    if (window.$dataWeapons) for (var w = 1; w < $dataWeapons.length; w++) {
      if ($dataWeapons[w]) $gameParty.gainItem($dataWeapons[w], 1);
    }
    if (window.$dataArmors) for (var a = 1; a < $dataArmors.length; a++) {
      if ($dataArmors[a]) $gameParty.gainItem($dataArmors[a], 1);
    }
    return true;
  };

  // God mode: max all stats for all party members
  window.__rtranslator_cheat_godMode = function() {
    if (!window.$gameParty) return false;
    var members = $gameParty.members();
    for (var i = 0; i < members.length; i++) {
      var m = members[i];
      m._paramPlus[2] = 9999; // ATK
      m._paramPlus[3] = 9999; // DEF
      m._paramPlus[4] = 9999; // MAT
      m._paramPlus[5] = 9999; // MDF
      m._paramPlus[6] = 9999; // AGI
      m._paramPlus[7] = 9999; // LUK
      m._hp = m.mhp; m._mp = m.mmp;
      m.refresh();
    }
    return true;
  };

  // Force win battle (proper way)
  window.__rtranslator_cheat_forceWin = function() {
    if (!window.BattleManager) return false;
    if (BattleManager._phase) {
      BattleManager.processVictory();
      return true;
    }
    return false;
  };

  // Force escape
  window.__rtranslator_cheat_forceEscape = function() {
    if (!window.BattleManager) return false;
    if (BattleManager._phase) {
      BattleManager.processEscape();
      return true;
    }
    return false;
  };

  // Walk through walls toggle
  window.__rtranslator_cheat_walkThroughWalls = function() {
    if (!window.$gamePlayer) return false;
    $gamePlayer._through = !$gamePlayer._through;
    return $gamePlayer._through;
  };

  // Level up all members
  window.__rtranslator_cheat_levelUp = function(levels) {
    if (!window.$gameParty) return false;
    levels = levels || 1;
    var members = $gameParty.members();
    for (var i = 0; i < members.length; i++) {
      var m = members[i];
      m._level = Math.min(99, m._level + levels);
      m.refresh();
    }
    return true;
  };

  // Get data lists (for UI)
  window.__rtranslator_getDataLists = function() {
    var lists = { actors: [], classes: [], items: [], weapons: [], armors: [], skills: [], states: [] };
    if (window.$dataActors) for (var i = 1; i < $dataActors.length; i++) {
      if ($dataActors[i] && $dataActors[i].name) lists.actors.push({id:i, name:$dataActors[i].name});
    }
    if (window.$dataItems) for (var i = 1; i < $dataItems.length; i++) {
      if ($dataItems[i] && $dataItems[i].name) lists.items.push({id:i, name:$dataItems[i].name});
    }
    if (window.$dataWeapons) for (var i = 1; i < $dataWeapons.length; i++) {
      if ($dataWeapons[i] && $dataWeapons[i].name) lists.weapons.push({id:i, name:$dataWeapons[i].name});
    }
    if (window.$dataArmors) for (var i = 1; i < $dataArmors.length; i++) {
      if ($dataArmors[i] && $dataArmors[i].name) lists.armors.push({id:i, name:$dataArmors[i].name});
    }
    return lists;
  };
})();
