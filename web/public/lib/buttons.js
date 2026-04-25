const updateSoundButtonIcon = (enabled) => {
  const topBar = document.getElementById('top-bar');
  if (!topBar || !topBar.children || !topBar.children[0]) {
    return;
  }
  const soundButton = topBar.children[0];
  const img = soundButton.children[0];
  if (!img) {
    return;
  }
  img.src = enabled ? 'icons/sound_on.svg' : 'icons/sound_off.svg';
};

window.Lib.subscribe('onToggleSound', updateSoundButtonIcon);
updateSoundButtonIcon(window.Lib.getConfig().soundEnabled);

window.onToggleSound = function () {
  window.Lib.toggleSound();
};

window.onToggleControls = function () {
  const controls = window.Lib.getConfig().showControls;
  if (controls) {
    window.Lib.hideControls();
    const gameOuter = document.getElementById('outer-game');
    gameOuter.classList.remove('game-outer-yes-controls');
    gameOuter.classList.add('game-outer-no-controls');
  } else {
    window.Lib.showControls();
    const gameOuter = document.getElementById('outer-game');
    gameOuter.classList.remove('game-outer-no-controls');
    gameOuter.classList.add('game-outer-yes-controls');
  }
};

window.onToggleHelp = function () {
  const url = `instructions/instructions.${
    window.Lib.getConfig().language
  }.html`;
  const iframe = document.createElement('iframe');
  iframe.src = url;
  iframe.id = 'instructions';
  iframe.className = 'instructions';

  const closeInstructions = function (e) {
    if (e.key === 'Escape') {
      document.getElementById('help-holder').remove();
      window.removeEventListener('keydown', closeInstructions);
    }
    e.stopPropagation();
  };
  window.addEventListener('keydown', closeInstructions);

  const closeButton = document.createElement('button');
  closeButton.id = 'close-instructions';
  closeButton.className = 'close-instructions cancel-button';
  closeButton.innerHTML = '<img src="icons/cancel.svg" alt="Close" />';

  const closeButtonHolder = document.createElement('div');
  closeButtonHolder.id = 'close-instructions-holder';
  closeButtonHolder.className = 'close-instructions-holder';
  closeButtonHolder.appendChild(closeButton);

  const iframeHolder = document.createElement('div');
  iframeHolder.id = 'help-holder';
  iframeHolder.className = 'help-holder';

  iframeHolder.appendChild(closeButtonHolder);
  iframeHolder.appendChild(iframe);

  closeButton.onclick = function () {
    document.getElementById('help-holder').remove();
  };

  document.body.appendChild(iframeHolder);
};
