(function() {
  if (window.__rtranslator_trans) return;
  window.__rtranslator_trans = {};

  var translationMap = {};

  window.__rtranslator_initTrans = function(map) { translationMap = map || {}; };

  function translate(text) {
    if (!text || typeof text !== 'string') return text;
    var translated = translationMap[text];
    return translated !== undefined ? translated : text;
  }

  window.__rtranslator_translate = translate;

  window.__rtranslator_installHooks = function() {
    if (!window.Window_Base) { setTimeout(window.__rtranslator_installHooks, 100); return; }

    if (window.Game_Message && window.Game_Message.prototype.add) {
      var _add = Game_Message.prototype.add;
      Game_Message.prototype.add = function(text) { _add.call(this, translate(text)); };
    }

    if (window.Window_Help && window.Window_Help.prototype.setText) {
      var _setText = Window_Help.prototype.setText;
      Window_Help.prototype.setText = function(text) { _setText.call(this, translate(text)); };
    }
  };
})();
