(function() {
  var WS_PORT = __RTRANSLATOR_WS_PORT__;

  var _d = window.__rtranslator_dappend || function(){};
  var _b = window.__rtranslator_dbadge || function(){};
  var mapSize = window.__RTRANSLATOR_PRELOAD_MAP__ ? Object.keys(window.__RTRANSLATOR_PRELOAD_MAP__).length : 0;
  _d('HOOK_LOADED map=' + mapSize + ' ws=' + WS_PORT);

  function start() {
    window.__rtranslator_connect(WS_PORT);

    window.__rtranslator_on('init', function(msg) {
      var sz = msg.map ? Object.keys(msg.map).length : 0;
      _d('INIT map=' + sz);
      _b('INIT' + sz);
      window.__rtranslator_send({ type: 'notify', event: 'init_received', count: sz });
      if (msg.map) window.__rtranslator_initTrans(msg.map);
      window.__rtranslator_installHooks();
      window.__rtranslator_send({ type: 'notify', event: 'hooks_installed' });
      _d('INIT_DONE');
      window.__rtranslator_dflush();
    });

    window.__rtranslator_on('read', function(msg) {
      var val = window.__rtranslator_readState(msg.path);
      window.__rtranslator_send({ type: 'state', path: msg.path, value: val });
    });

    window.__rtranslator_on('write', function(msg) {
      var ok = window.__rtranslator_writeState(msg.path, msg.value);
      window.__rtranslator_send({ type: 'ack', cmd: 'write', success: ok });
    });

    window.__rtranslator_on('batch', function(msg) {
      for (var i = 0; i < (msg.ops || []).length; i++) {
        var op = msg.ops[i];
        if (op.type === 'write') window.__rtranslator_writeState(op.path, op.value);
      }
      window.__rtranslator_send({ type: 'ack', cmd: 'batch', success: true });
    });

    window.__rtranslator_on('battle_victory', function() {
      window.__rtranslator_cheat_forceWin();
      window.__rtranslator_send({ type: 'ack', cmd: 'battle_victory', success: true });
    });

    window.__rtranslator_on('cheat', function(msg) {
      var action = msg.action;
      var ok = false;
      switch (action) {
        case 'recover_all': ok = window.__rtranslator_cheat_recoverAll(); break;
        case 'max_gold': ok = window.__rtranslator_cheat_maxGold(); break;
        case 'all_items': ok = window.__rtranslator_cheat_allItems(); break;
        case 'god_mode': ok = window.__rtranslator_cheat_godMode(); break;
        case 'force_escape': ok = window.__rtranslator_cheat_forceEscape(); break;
        case 'walk_walls': ok = window.__rtranslator_cheat_walkThroughWalls(); break;
        case 'level_up': ok = window.__rtranslator_cheat_levelUp(msg.levels || 1); break;
        case 'get_lists':
          var data = window.__rtranslator_getDataLists();
          window.__rtranslator_send({ type: 'data_lists', data: data });
          return;
      }
      window.__rtranslator_send({ type: 'ack', cmd: 'cheat', action: action, success: ok });
    });

    window.__rtranslator_on('add_item', function(msg) {
      if (window.$gameParty) {
        var data = null;
        if (msg.itemType === 'item' && window.$dataItems) data = window.$dataItems[msg.id];
        else if (msg.itemType === 'weapon' && window.$dataWeapons) data = window.$dataWeapons[msg.id];
        else if (msg.itemType === 'armor' && window.$dataArmors) data = window.$dataArmors[msg.id];
        if (data) { window.$gameParty.gainItem(data, msg.count || 1); }
        window.__rtranslator_send({ type: 'ack', cmd: 'add_item', success: !!data });
      }
    });

    window.__rtranslator_on('reload_trans', function(msg) {
      var sz = msg.map ? Object.keys(msg.map).length : 0;
      _d('RELOAD_TRANS map=' + sz);
      _b('RLD' + sz);
      if (msg.map) { window.__rtranslator_initTrans(msg.map); }
      window.__rtranslator_send({ type: 'ack', cmd: 'reload_trans', success: true });
    });
  }

  var tries = 0;
  function waitForReady() {
    tries++;
    if (window.Window_Base || tries > 150) {
      _d('START tries=' + tries + ' hasWB=' + !!window.Window_Base);
      _b('START');
      start();
    }
    else setTimeout(waitForReady, 200);
  }
  waitForReady();
})();
