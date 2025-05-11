const canvas = document.getElementById('canvas');
canvas.width = 512;
canvas.height = 512;
const params = new URLSearchParams(window.location.search);

// params
const language = params.get('language') || 'en';
const isArcadeCabinet = params.get('arcade') === 'true';
const shouldMute = params.get('muted') === 'true';
const tapToStart = params.get('tap');

const subscriptions = {
  onButtonDown: [],
  onButtonUp: [],
  onToggleModuleControls: [],
  onToggleSound: [],
  onSetVolume: [],
};

const config = {
  gameStarted: false,
  soundEnabled: true,
  disableInput: false,
  isArcadeCabinet,
  language,
  targetWidth: 512,
  targetHeight: 512,
  showControls: true,
};

const Lib = {
  getConfig() {
    return config;
  },
  getCanvas() {
    return canvas;
  },
  hideLoading() {
    document.getElementById('loading').style.display = 'none';
  },
  showError() {
    document.getElementById('loading').style.display = 'none';
  },
  showGame() {
    document.getElementById('game').style.display = 'flex';
    document.getElementById('top-bar').style.display = 'flex';
  },
  hideGame() {
    document.getElementById('game').style.display = 'none';
  },
  showControls() {
    document.getElementById('controls').style.display = 'block';
    config.showControls = true;
  },
  hideControls() {
    document.getElementById('controls').style.display = 'none';
    config.showControls = false;
  },
  disableModuleControls() {
    Module.ccall('setKeyStatus', 'void', ['number'], [0]);
    this.invokeEvent('onToggleModuleControls', false);
  },
  enableModuleControls() {
    Module.ccall('setKeyStatus', 'void', ['number'], [1]);
    this.invokeEvent('onToggleModuleControls', true);
  },
  toggleSound() {
    if (config.soundEnabled) {
      Module.ccall('disableSound');
    } else {
      Module.ccall('enableSound');
    }
    config.soundEnabled = !config.soundEnabled;
    this.invokeEvent('onToggleSound', config.soundEnabled);
  },
  setWASMVolume(pct) {
    if (isNaN(pct)) {
      pct = 33;
    }
    Module.ccall('setVolume', 'void', ['number'], [pct]);
    this.invokeEvent('onSetVolume', pct);
  },
  handleButtonDown(key) {
    if (config.disableInput) {
      return;
    }
    if (window.event) {
      window.event.preventDefault();
    }
    Module.ccall('setKeyDown', 'void', ['number'], [key]);
    this.invokeEvent('onButtonDown', key);
  },
  handleButtonUp(key) {
    if (config.disableInput) {
      return;
    }
    if (window.event) {
      window.event.preventDefault();
    }
    Module.ccall('setKeyUp', 'void', ['number'], [key]);
    this.invokeEvent('onButtonUp', key);
  },
  notifyParentFrame(action, payload) {
    if (window.parent) {
      console.log('[LIB] Notify parent', action);
      window.parent.postMessage(
        JSON.stringify({
          action,
          payload,
        }),
        '*'
      );
    }
  },
  notifyTargetWindowSize(w, h) {
    console.log('[LIB] Notify target window size', w, h);
    config.targetWidth = w;
    config.targetHeight = h;
    canvas.width = w;
    canvas.height = h;
    this.notifyParentFrame('TARGET_WINDOW_SIZE', { w, h });
  },
  notifyGameReady() {
    // wait just a bit to show the game so the audio doesn't glitch out (like it does for some reason for wasm stuff that has debug on)
    this.setWASMVolume(33);
    if (Lib.getConfig().autoPushStart) {
      setTimeout(() => {
        this.handleButtonDown(this.BUTTON_ENTER);
        this.handleButtonUp(this.BUTTON_ENTER);
      }, 500);
    }
    this.notifyParentFrame('GAME_READY', {});
  },
  notifyGameStarted() {
    this.notifyParentFrame('GAME_STARTED', {});
    config.gameStarted = true;
  },
  notifyGameCompleted(...results) {
    this.notifyParentFrame(
      'GAME_CONCLUDED',
      results.length > 1 ? results : results[0]
    );
    config.gameStarted = false;
  },
  subscribe(eventName, callback) {
    if (subscriptions[eventName]) {
      subscriptions[eventName].push(callback);
    }
  },
  unsubscribe(eventName, callback) {
    if (subscriptions[eventName]) {
      subscriptions[eventName] = subscriptions[eventName].filter(
        (cb) => cb !== callback
      );
    }
  },
  invokeEvent(eventName, ...args) {
    if (subscriptions[eventName]) {
      subscriptions[eventName].forEach((cb) => cb(...args));
    }
  },
};
window.Lib = Lib;

// Emscripten module
var Module = {
  arguments: ['--language ' + language],
  jsLoaded: function () {
    Module.preRun[0]();
  },
  preRun: [
    function () {
      clearTimeout(window.loadTimeout);
    },
  ],
  postRun: [
    function () {
      Lib.hideLoading();
      Lib.showGame();

      if (shouldMute) {
        // HACK: delayed because the WASM is sometimes not ready to accept the call
        // to mute it
        setTimeout(() => {
          Lib.getConfig().soundEnabled = true;
          Lib.toggleSound();
        }, 300);
      }

      if (isArcadeCabinet) {
        Lib.disableModuleControls();
      }
    },
  ],
  canvas: (function () {
    if (canvas) {
      canvas.addEventListener(
        'webglcontextlost',
        function (e) {
          console.error(
            '[LIB] WebGL context lost. You will need to reload the page.'
          );
          Lib.hideLoading();
          Lib.showError();
          e.preventDefault();
        },
        false
      );
      if (isArcadeCabinet) {
        canvas.style.border = 'unset';
      }
    }

    return canvas;
  })(),
  onAbort: function () {
    console.error('[LIB] Program encountered an unknown error.');
    Lib.showError();
    Lib.hideLoading();
    Lib.hideGame();
  },
  totalDependencies: 0,
  ccall: function () {},
};
window.Module = Module;

function verify() {
  if (!window.init) {
    console.error('[LIB] no `init` found.');
  }
  if (!window.start) {
    console.error('[LIB] no `start` found.');
  }
}

const localInit = async () => {
  verify();
  window.loadTimeout = setTimeout(function () {
    console.error('[LIB] Content took too long to load.');
    Lib.showError();
  }, 60000);

  try {
    await window.init({
      params,
      gameType: params.get('gameType'),
    });
    Lib.showControls();
  } catch (e) {
    console.error(
      '[LIB] Error calling window.init function, is it defined for this program?'
    );
    throw e;
  }
};

if (tapToStart === 'true') {
  const div = document.createElement('div');
  const loadingElem = document.getElementById('loading');
  if (loadingElem) {
    loadingElem.style.display = 'none';
  }
  window.onTapToStart = function () {
    window.removeEventListener('mousedown', window.onTapToStart);
    window.removeEventListener('touchstart', window.onTapToStart);
    if (loadingElem) {
      loadingElem.style.display = 'flex';
    }
    div.style.display = 'none';
    localInit();
  };
  div.innerHTML = Lib.getLabels().tapToStart;
  div.className = 'tap-to-start';
  document.body.appendChild(div);
  window.addEventListener('mousedown', window.onTapToStart);
  window.addEventListener('touchstart', window.onTapToStart);
} else {
  window.addEventListener('load', localInit);
}

// required for wasm to grab keyboard controls
setInterval(() => {
  if (window.parent) {
    if (Lib.getConfig().gameStarted) {
      window.focus();
    }
  }
}, 500);
