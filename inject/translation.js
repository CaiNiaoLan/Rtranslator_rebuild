(function() {
  if (window.__rtranslator_trans) return;
  window.__rtranslator_trans = {};

  var translationMap = {};

  window.__rtranslator_initTrans = function(map) {
    translationMap = map || {};
    translateTextManager();
    translateExistingData();
  };

  // Translate TextManager templates immediately
  function translateTextManager() {
    if (window.TextManager) {
      for (var tk in TextManager) {
        if (typeof TextManager[tk] === 'string' && TextManager[tk].length > 0) {
          TextManager[tk] = translate(TextManager[tk]);
        }
      }
    }
  }

  // Also re-translate already-loaded data objects if they exist
  function translateExistingData() {
    var dataObjects = ['$dataActors','$dataClasses','$dataSkills','$dataItems',
      '$dataWeapons','$dataArmors','$dataEnemies','$dataTroops','$dataStates',
      '$dataAnimations','$dataTilesets','$dataCommonEvents','$dataSystem','$dataMapInfos'];
    for (var d = 0; d < dataObjects.length; d++) {
      if (window[dataObjects[d]]) translateRecursive(window[dataObjects[d]]);
    }
  }

  function translate(text) {
    if (!text || typeof text !== 'string') return text;
    var translated = translationMap[text];
    return translated !== undefined ? translated : text;
  }

  window.__rtranslator_translate = translate;

  // === HOOK 1: JSON.parse — intercept ALL game data at load time ===
  var _jsonParse = JSON.parse;
  JSON.parse = function(text, reviver) {
    var obj = _jsonParse(text, reviver);
    if (obj && typeof obj === 'object') translateRecursive(obj);
    return obj;
  };

  function translateRecursive(obj) {
    if (!obj || typeof obj !== 'object') return;
    if (Array.isArray(obj)) {
      for (var i = 0; i < obj.length; i++) translateRecursive(obj[i]);
      return;
    }
    for (var key in obj) {
      if (!obj.hasOwnProperty(key)) continue;
      var val = obj[key];
      if (typeof val === 'string' && val.length > 0) {
        if (!/^[0-9+\-*/\s().,%]+$/.test(val)) {
          obj[key] = translate(val);
        }
      } else if (typeof val === 'object' && val !== null) {
        translateRecursive(val);
      }
    }
  }

  window.__rtranslator_installHooks = function() {
    if (window.__rtranslator_hooksInstalled) return;
    window.__rtranslator_hooksInstalled = true;

    // === Canvas fillText ===
    try {
      var proto = CanvasRenderingContext2D && CanvasRenderingContext2D.prototype;
      if (proto && proto.fillText) {
        var _fillText = proto.fillText;
        proto.fillText = function(text, x, y, maxWidth) {
          if (arguments.length > 3) _fillText.call(this, translate(String(text)), x, y, maxWidth);
          else _fillText.call(this, translate(String(text)), x, y);
        };
      }
    } catch(e) {}

    // === PIXI.Text for MZ WebGL ===
    try {
      if (window.PIXI && window.PIXI.Text && window.PIXI.Text.prototype.updateText) {
        var _pixiUpdate = window.PIXI.Text.prototype.updateText;
        window.PIXI.Text.prototype.updateText = function() {
          if (this._text && typeof this._text === 'string') this._text = translate(this._text);
          return _pixiUpdate.call(this);
        };
      }
    } catch(e) {}

    // === TextManager templates ===
    if (window.TextManager) {
      for (var tk in TextManager) {
        if (typeof TextManager[tk] === 'string' && TextManager[tk].length > 0) {
          TextManager[tk] = translate(TextManager[tk]);
        }
      }
    }

    // === DataManager.makeSavename — "File 1" labels ===
    if (window.DataManager && DataManager.makeSavename) {
      var _makeSavename = DataManager.makeSavename;
      DataManager.makeSavename = function(savefileId) {
        return translate(_makeSavename.call(this, savefileId));
      };
    }

    // === Window hooks ===
    installWindowHooks();
  };

  function installWindowHooks() {
    if (!window.Window_Base) { setTimeout(installWindowHooks, 100); return; }

    if (Window_Base.prototype.drawText) {
      var _wdt = Window_Base.prototype.drawText;
      Window_Base.prototype.drawText = function(text, x, y, maxWidth, align) {
        _wdt.call(this, translate(text), x, y, maxWidth, align);
      };
    }
    if (Window_Base.prototype.drawTextEx) {
      var _dtex = Window_Base.prototype.drawTextEx;
      Window_Base.prototype.drawTextEx = function(text, x, y) {
        _dtex.call(this, translate(text), x, y);
      };
    }
    if (window.Game_Message && Game_Message.prototype.add) {
      var _add = Game_Message.prototype.add;
      Game_Message.prototype.add = function(text) { _add.call(this, translate(text)); };
    }
    if (window.Window_Help && Window_Help.prototype.setText) {
      var _setText = Window_Help.prototype.setText;
      Window_Help.prototype.setText = function(text) { _setText.call(this, translate(text)); };
    }
    if (window.Window_Message && Window_Message.prototype.newPage) {
      var _newPage = Window_Message.prototype.newPage;
      Window_Message.prototype.newPage = function(text) { _newPage.call(this, translate(text)); };
    }
    if (window.Bitmap && Bitmap.prototype.drawText) {
      var _bdt = Bitmap.prototype.drawText;
      Bitmap.prototype.drawText = function(text, x, y, maxWidth, lineHeight, align) {
        _bdt.call(this, translate(text), x, y, maxWidth, lineHeight, align);
      };
    }
    if (window.Bitmap && Bitmap.prototype.measureTextWidth) {
      var _mtw = Bitmap.prototype.measureTextWidth;
      Bitmap.prototype.measureTextWidth = function(text) { return _mtw.call(this, translate(text)); };
    }
  }
})();
