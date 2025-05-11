window.onToggleSound = function (btn) {
  console.log('Toggle sound');
  window.Lib.toggleSound();
  const img = btn.children[0];
  if (window.Lib.getConfig().soundEnabled) {
    img.src = 'icons/sound_on.svg';
  } else {
    img.src = 'icons/sound_off.svg';
  }
};

window.onToggleControls = function () {
  console.log('Toggle controls');

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
