(function() {
  var WS_PORT = __RTRANSLATOR_WS_PORT__;

  function start() {
    window.__rtranslator_connect(WS_PORT);

    window.__rtranslator_on('init', function(msg) {
      if (msg.map) window.__rtranslator_initTrans(msg.map);
      window.__rtranslator_installHooks();
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
      if (window.$gameParty && window.$gameParty.inBattle && window.$gameParty.inBattle()) {
        var members = window.$gameTroop ? window.$gameTroop.members() : [];
        for (var i = 0; i < members.length; i++) members[i].setHp(0);
        window.__rtranslator_send({ type: 'ack', cmd: 'battle_victory', success: true });
      } else {
        window.__rtranslator_send({ type: 'error', cmd: 'battle_victory', reason: 'not in battle' });
      }
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
      if (msg.map) { window.__rtranslator_initTrans(msg.map); }
      window.__rtranslator_send({ type: 'ack', cmd: 'reload_trans', success: true });
    });
  }

  var tries = 0;
  function waitForReady() {
    tries++;
    if (window.Window_Base || tries > 50) start();
    else setTimeout(waitForReady, 200);
  }
  waitForReady();
})();
