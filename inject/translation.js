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
    if (window.__rtranslator_hooksInstalled) return;
    if (!window.Window_Base) { setTimeout(window.__rtranslator_installHooks, 100); return; }

    // Primary hook: Window_Base.prototype.drawText catches ALL text rendering
    if (Window_Base.prototype.drawText) {
        var _drawText = Window_Base.prototype.drawText;
        Window_Base.prototype.drawText = function(text, x, y, maxWidth, align) {
            _drawText.call(this, translate(text), x, y, maxWidth, align);
        };
    }

    // Secondary: Game_Message.add for message text (caught by drawText too, but belt-and-suspenders)
    if (window.Game_Message && Game_Message.prototype.add) {
        var _add = Game_Message.prototype.add;
        Game_Message.prototype.add = function(text) {
            _add.call(this, translate(text));
        };
    }

    // Choice text: Window_ChoiceList.drawItem processes choices before drawText
    // Window_Command derivatives use drawText internally, so the drawText hook covers them

    window.__rtranslator_hooksInstalled = true;
  };
})();
