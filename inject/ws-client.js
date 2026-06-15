(function() {
  if (window.__rtranslator_ws) return;
  window.__rtranslator_ws = null;
  window.__rtranslator_callbacks = {};

  function connect(port) {
    var ws = new WebSocket('ws://127.0.0.1:' + port);
    ws.onopen = function() {};
    ws.onmessage = function(event) {
      try { var msg = JSON.parse(event.data); handleMessage(msg); } catch(e) {}
    };
    ws.onclose = function() { window.__rtranslator_ws = null; };
    window.__rtranslator_ws = ws;
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
