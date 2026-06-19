(function() {
  if (window.__rtranslator_ws) return;
  window.__rtranslator_ws = null;
  window.__rtranslator_callbacks = {};

  // Shared diagnostic log
  window.__rtranslator_dlog = [];
  window.__rtranslator_dappend = function(msg) { window.__rtranslator_dlog.push(msg); };
  window.__rtranslator_dflush = function() {
    try {
      var fs = require('fs'), path = require('path');
      fs.writeFileSync(path.join(process.cwd(), 'rtranslator.log'), window.__rtranslator_dlog.join('\n'), 'utf-8');
    } catch(e) {}
  };

  // Visible badge
  var diag = document.createElement('div');
  diag.id = '__rtl_diag';
  diag.style.cssText = 'position:fixed;top:4px;right:4px;background:rgba(0,0,0,0.7);color:#0f0;padding:3px 8px;font-size:10px;z-index:99999;border-radius:3px;font-family:monospace';
  diag.textContent = 'RTL:LOADED';
  window.__rtranslator_dbadge = function(t) { diag.textContent = 'RTL:' + t; };
  try { if (document.body) document.body.appendChild(diag); else document.addEventListener('DOMContentLoaded', function() { if (document.body) document.body.appendChild(diag); }); } catch(e) {}

  window.__rtranslator_dappend('WS_LOADED');

  function connect(port) {
    window.__rtranslator_dappend('WS_CONNECT port=' + port);
    function tryConnect() {
      var ws = new WebSocket('ws://127.0.0.1:' + port);
      ws.onopen = function() {
        window.__rtranslator_ws = ws;
        window.__rtranslator_dappend('WS_OPEN');
        window.__rtranslator_dbadge('WS_OPEN');
      };
      ws.onmessage = function(event) {
        try { var msg = JSON.parse(event.data); handleMessage(msg); } catch(e) { window.__rtranslator_dappend('WS_MSG_ERR ' + e.message); }
      };
      ws.onclose = function() {
        window.__rtranslator_ws = null;
        window.__rtranslator_dappend('WS_CLOSE');
        setTimeout(tryConnect, 2000);
      };
      ws.onerror = function(e) { window.__rtranslator_dappend('WS_ERROR'); };
    }
    tryConnect();
  }

  function send(data) {
    if (window.__rtranslator_ws && window.__rtranslator_ws.readyState === WebSocket.OPEN) {
      window.__rtranslator_ws.send(JSON.stringify(data));
    }
  }

  function on(type, callback) {
    if (!window.__rtranslator_callbacks[type]) window.__rtranslator_callbacks[type] = [];
    window.__rtranslator_callbacks[type].push(callback);
  }

  function handleMessage(msg) {
    var cbs = window.__rtranslator_callbacks[msg.type] || [];
    for (var i = 0; i < cbs.length; i++) { cbs[i](msg); }
  }

  window.__rtranslator_connect = connect;
  window.__rtranslator_send = send;
  window.__rtranslator_on = on;
})();
