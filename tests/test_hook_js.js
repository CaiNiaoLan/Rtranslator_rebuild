// Mock browser globals — put RPG Maker globals on both global and window
global.$gameParty = {
  _gold: 500, gold: function() { return this._gold; }, steps: function() { return 42; },
  gainItem: function() {}, loseItem: function() {}, numItems: function() { return 3; }
};
global.$gameSwitches = { _data: {}, value: function(id) { return this._data[id] || false; } };
global.$gameVariables = { _data: {}, value: function(id) { return this._data[id] || 0; } };
global.$gameActors = {
  actor: function(id) {
    return id === 1 ? { hp: 100, mp: 50, _level: 5, name: function(){return "Hero";},
      atk:20, def:15, mat:10, mdf:10, agi:12, luk:8, mhp:100, mmp:50, currentExp: function(){return 0;} } : null;
  }
};
global.$dataItems = [];

global.window = {
  $gameParty: global.$gameParty,
  $gameSwitches: global.$gameSwitches,
  $gameVariables: global.$gameVariables,
  $gameActors: global.$gameActors,
  $dataItems: global.$dataItems
};

require('../inject/state-reader.js');

var errors = [];
function assert(condition, msg) { if (!condition) errors.push('FAIL: ' + msg); }

assert(window.__rtranslator_readState('party.gold') === 500, 'party.gold');
assert(window.__rtranslator_readState('actors.1.hp') === 100, 'actors.1.hp');

window.__rtranslator_writeState('party.gold', 9999);
assert(global.$gameParty._gold === 9999, 'gold write');

window.__rtranslator_writeState('actors.1.hp', 50);

if (errors.length === 0) { console.log('All JS hook tests PASSED'); process.exit(0); }
else { console.error(errors.join('\n')); process.exit(1); }
