(function() {
  if (window.__rtranslator_trans) return;
  window.__rtranslator_trans = {};

  var _d = window.__rtranslator_dappend || function(){};
  var _b = window.__rtranslator_dbadge || function(){};
  var translationMap = window.__RTRANSLATOR_PRELOAD_MAP__ || {};
  var preloadCount = Object.keys(translationMap).length;
  _d('TRANS_PRELOAD map=' + preloadCount);
  _b('MAP' + preloadCount);
  delete window.__RTRANSLATOR_PRELOAD_MAP__;

  // If preloaded, initialize immediately (before game data loads and windows render)
  if (preloadCount > 0) {
    _d('TRANS_PRELOAD_INIT');
    translateTextManager();
    _d('TRANS_TEXTMGR_DONE');
    installWindowHooks();
    _d('TRANS_HOOKS_DONE');
  } else {
    _d('TRANS_NO_PRELOAD');
  }

  window.__rtranslator_initTrans = function(map) {
    translationMap = map || {};
    _buildIndexes();
    translateTextManager();
    translateExistingData();
    // Force scene refresh to apply new translations immediately
    try {
      if (typeof SceneManager !== 'undefined' && SceneManager._scene) {
        var ctor = SceneManager._scene.constructor;
        SceneManager.goto(ctor);
      }
    } catch(e) { _d('REFRESH_ERR:' + e.message); }
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

  // Build secondary indexes for fuzzy matching
  var _escapedMap = {}; // key: stripped-of-escape-codes text → translation
  var _lineMap = {};    // key: single line → per-line translation
  var _prefixIdx = {};  // key: first 2 chars → array of candidate keys (for substring matching)
  var _maxSubScan = 200;
  function _buildIndexes() {
    _escapedMap = {}; _lineMap = {}; _prefixIdx = {};
    var keys = Object.keys(translationMap);
    for (var i = 0; i < keys.length; i++) {
      var k = keys[i], v = translationMap[k];
      // Strip RPG Maker escape codes for normalized matching
      var stripped = k.replace(/\\[VvNnCcPp]\s*\[\s*\d+\s*\]/g, '')
                      .replace(/\\[Gg]/g, '')
                      .replace(/\\[{}]/g, '').trim();
      if (stripped && !_escapedMap[stripped]) _escapedMap[stripped] = v;

      // Split multi-line keys into per-line entries
      if (k.indexOf('\n') >= 0) {
        var kl = k.split('\n'), vl = v.split('\n');
        var len = Math.min(kl.length, vl.length);
        for (var j = 0; j < len; j++) {
          var rawLine = kl[j], vt = vl[j].trim();
          var trimmedKey = rawLine.trim();
          if (trimmedKey && !_lineMap[trimmedKey]) _lineMap[trimmedKey] = vt;
          if (rawLine !== trimmedKey && rawLine && !_lineMap[rawLine]) _lineMap[rawLine] = vt;
        }
      }

      // Prefix index for fast substring search
      if (k.length >= 2) {
        var pf = k.substring(0, 2);
        if (!_prefixIdx[pf]) _prefixIdx[pf] = [];
        if (_prefixIdx[pf].length < _maxSubScan) _prefixIdx[pf].push(k);
      }
    }
  }
  _buildIndexes();

  var _totalLookups = 0, _totalHits = 0, _sampleMisses = [];
  function translate(text) {
    if (!text || typeof text !== 'string') return text;
    _totalLookups++;
    var result;

    // Strategy 1: Exact match (fast path)
    result = translationMap[text];
    if (result !== undefined) { _totalHits++; return result; }

    // Strategy 2: Trimmed exact match
    var trimmed = text.trim();
    if (trimmed !== text) {
      result = translationMap[trimmed];
      if (result !== undefined) { _totalHits++; return result; }
    }

    // Strategy 3: Line match (multi-line keys rendered one line at a time)
    if ((result = _lineMap[text]) !== undefined) { _totalHits++; return result; }
    if (trimmed !== text && (result = _lineMap[trimmed]) !== undefined) { _totalHits++; return result; }

    // Strategy 4: Strip escape codes and match normalized map
    var stripped = text.replace(/\\[VvNnCcPp]\s*\[\s*\d+\s*\]/g, '')
                       .replace(/\\[Gg]/g, '')
                       .replace(/\\[{}]/g, '').trim();
    if (stripped !== text) {
      result = translationMap[stripped];
      if (result !== undefined) { _totalHits++; return result; }
      result = _escapedMap[stripped];
      if (result !== undefined) { _totalHits++; return result; }
    }

    // Strategy 5: Substring containment using prefix index (only short texts)
    // Game window wrapping produces short fragments; try matching as substring of longer keys
    if (text.length >= 2 && text.length <= 80) {
      var pf = text.substring(0, 2);
      var candidates = _prefixIdx[pf] || [];
      for (var i = 0; i < candidates.length; i++) {
        if (candidates[i].indexOf(text) >= 0) { _totalHits++; return translationMap[candidates[i]]; }
      }
    }

    // Strategy 6: Text might CONTAIN a key (e.g. variable-substituted text contains a template)
    // Only when text is short enough to be manageable
    if (text.length <= 120) {
      var keys = Object.keys(translationMap);
      for (var i = 0; i < keys.length && i < _maxSubScan; i++) {
        if (text.indexOf(keys[i]) >= 0) { _totalHits++; return translationMap[keys[i]]; }
      }
    }

    // Track misses
    if (_sampleMisses.length < 10 && text.length > 1 && !/^[\s\d]+$/.test(text)) {
      _sampleMisses.push(text.substring(0,40));
    }
    return text;
  }

  window.__rtranslator_translate = translate;
  window.__rtranslator_transStats = function() {
    return 'lookups=' + _totalLookups + ' hits=' + _totalHits + ' sampleMisses=' + JSON.stringify(_sampleMisses);
  };

  // === HOOK 1: JSON.parse — intercept ALL game data at load time ===
  var _jsonParse = JSON.parse;
  var parseHookCount = 0;
  JSON.parse = function(text, reviver) {
    var obj = _jsonParse(text, reviver);
    if (obj && typeof obj === 'object') {
      parseHookCount++;
      if (parseHookCount === 1) { _d('JSON_PARSE_HOOK_FIRED map=' + Object.keys(translationMap).length); _b('PHK' + Object.keys(translationMap).length); }
      translateRecursive(obj);
    }
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

    _d('HOOKS_INSTALLING hasDrawText=' + !!Window_Base.prototype.drawText);
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
