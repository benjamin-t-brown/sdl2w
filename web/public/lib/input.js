// SDL2 Button Codes for emcc/gcc compiler
const BUTTON_LEFT = 1073741904;
const BUTTON_RIGHT = 1073741903;
const BUTTON_UP = 1073741906;
const BUTTON_DOWN = 1073741905;
const BUTTON_SHIFT = 1073742049; //225
const BUTTON_CTRL = 1073742048; //224
const BUTTON_ENTER = 13;
const BUTTON_SPACE = 32; //44
const BUTTON_X = 120; //27
const BUTTON_Z = 122; //29
const BUTTON_C = 99; //6

window.Lib.getButtons = () => {
  return {
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SHIFT,
    BUTTON_CTRL,
    BUTTON_ENTER,
    BUTTON_SPACE,
    BUTTON_X,
    BUTTON_Z,
    BUTTON_C,
  };
};

const gamepadButtonsState = new Map();
const simulateKeyPress = (buttonIndex, isDown) => {
  const gamepadButtonIndexToKey = {
    0: 'x',
    1: 'z',
    2: 'c',
    3: 'y',
    4: 'left bumper',
    5: 'right bumper',
    6: 'shift',
    7: 'right trigger',
    8: 'Backspace',
    9: ' ',
    10: 'left stick',
    11: 'right stick',
    12: 'ArrowUp',
    13: 'ArrowDown',
    14: 'ArrowLeft',
    15: 'ArrowRight',
    16: 'home',
  };
  const gamepadButtonToLibKey = {
    0: BUTTON_SPACE,
    1: BUTTON_SHIFT,
    2: BUTTON_UP,
    3: BUTTON_C,
    4: BUTTON_CTRL,
    5: BUTTON_Z,
    6: BUTTON_X,
    7: 'right trigger',
    8: 'Backspace',
    9: ' ',
    10: 'left stick',
    11: 'right stick',
    12: BUTTON_UP,
    13: BUTTON_DOWN,
    14: BUTTON_LEFT,
    15: BUTTON_RIGHT,
    16: BUTTON_ENTER,
  };

  const key = gamepadButtonIndexToKey[buttonIndex];
  if (!key) {
    return;
  }

  const simulatedEvent = new KeyboardEvent(isDown ? 'keydown' : 'keyup', {
    key,
  });
  window.dispatchEvent(simulatedEvent);
  if (isDown) {
    window.Lib.handleButtonDown(
      gamepadButtonToLibKey[buttonIndex],
      simulatedEvent
    );
  } else {
    window.Lib.handleButtonUp(gamepadButtonToLibKey[buttonIndex]);
  }
};

const getAxisButtonPress = (axisValue, button1I, button2I) => {
  if (axisValue < -0.75) {
    return button1I;
  } else if (axisValue > 0.75) {
    return button2I;
  }
  return null;
};

const updateGamePad = (gamepad) => {
  const buttonsToUnpress = [];
  const buttonsToSkipUnpress = [];
  for (const [i, button] of gamepad.buttons.entries()) {
    const pctPressed = button.value * 100;
    if (button.pressed && pctPressed > 50) {
      buttonsToSkipUnpress.push(i);
      if (!gamepadButtonsState.get(i)) {
        gamepadButtonsState.set(i, true);
        simulateKeyPress(i, true);
      }
    } else {
      buttonsToUnpress.push(i);
    }
  }

  for (const [i, axisValue] of gamepad.axes.entries()) {
    let localButtonsToUnpress = [];
    // left stick x
    if (i === 0) {
      const leftButtonI = 14;
      const rightButtonI = 15;

      const buttonI = getAxisButtonPress(axisValue, leftButtonI, rightButtonI);
      if (buttonI !== null) {
        if (!gamepadButtonsState.get(buttonI)) {
          gamepadButtonsState.set(buttonI, true);
          simulateKeyPress(buttonI, true);
        }
        buttonsToSkipUnpress.push(buttonI);
        localButtonsToUnpress.push(
          buttonI === leftButtonI ? rightButtonI : leftButtonI
        );
      } else if (buttonI === null) {
        localButtonsToUnpress.push(leftButtonI);
        localButtonsToUnpress.push(rightButtonI);
      }
    }
    // left stick y
    if (i === 1) {
      const upButtonI = 12;
      const downButtonI = 13;

      const buttonI = getAxisButtonPress(axisValue, upButtonI, downButtonI);
      if (buttonI !== null) {
        if (!gamepadButtonsState.get(buttonI)) {
          gamepadButtonsState.set(buttonI, true);
          simulateKeyPress(buttonI, true);
        }
        buttonsToSkipUnpress.push(buttonI);
        localButtonsToUnpress.push(
          buttonI === upButtonI ? downButtonI : upButtonI
        );
      } else if (buttonI === null) {
        localButtonsToUnpress.push(upButtonI);
        localButtonsToUnpress.push(downButtonI);
      }
    }
    for (const j of localButtonsToUnpress) {
      if (gamepadButtonsState.get(i)) {
        buttonsToUnpress.push(j);
      }
    }
  }

  for (const i of buttonsToUnpress) {
    if (buttonsToSkipUnpress.includes(i)) {
      continue;
    }
    if (gamepadButtonsState.get(i)) {
      // button was released
      gamepadButtonsState.set(i, false);
      simulateKeyPress(i, false);
    }
  }
};

window.gamepadInterval = setInterval(() => {
  if (
    window.Lib.getConfig().gameStarted ||
    !window.Lib.getConfig().isArcadeCabinet
  ) {
    for (const gamepad of navigator.getGamepads()) {
      if (!gamepad) {
        continue;
      }
      updateGamePad(gamepad);
    }
  }
}, 16);

const buttonCallbackStrings = {
  Up: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_UP)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_UP)',
  },
  Down: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_DOWN)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_DOWN)',
  },
  Left: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_LEFT)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_LEFT)',
  },
  Right: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_RIGHT)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_RIGHT)',
  },
  Confirm: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_SPACE)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_SPACE)',
  },
  Cancel: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_Z)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_Z)',
  },
  Shift: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_SHIFT)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_SHIFT)',
  },
  Ctrl: {
    down: 'Lib.handleButtonDown(Lib.getButtons().BUTTON_CTRL)',
    up: 'Lib.handleButtonUp(Lib.getButtons().BUTTON_CTRL)',
  },
};

const controlsDpadNormal = `
<div class="scaffold-dpad">
  <div class="scaffold-flex-row-center">
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Up.down}"
      ontouchstart="${buttonCallbackStrings.Up.down}"
      onmouseup="${buttonCallbackStrings.Up.up}"
      ontouchend="${buttonCallbackStrings.Up.up}"
      >↑</button>
  </div>
  <div class="scaffold-flex-row-space-between">
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Left.down}"
      ontouchstart="${buttonCallbackStrings.Left.down}"
      onmouseup="${buttonCallbackStrings.Left.up}"
      ontouchend="${buttonCallbackStrings.Left.up}"
      >←</button>
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Right.down}"
      ontouchstart="${buttonCallbackStrings.Right.down}"
      onmouseup="${buttonCallbackStrings.Right.up}"
      ontouchend="${buttonCallbackStrings.Right.up}"
      >→</button>  
  </div>
  <div class="scaffold-flex-row-center">
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Down.down}"
      ontouchstart="${buttonCallbackStrings.Down.down}"
      onmouseup="${buttonCallbackStrings.Down.up}"
      ontouchend="${buttonCallbackStrings.Down.up}"
      >↓</button>
  </div>
</div>
`;

const controlsDpadLr = `
<div class="scaffold-dpad">
  <div class="scaffold-flex-row-space-between">
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Left.down}"
      ontouchstart="${buttonCallbackStrings.Left.down}"
      onmouseup="${buttonCallbackStrings.Left.up}"
      ontouchend="${buttonCallbackStrings.Left.up}"
      >←</button>
    <button class="scaffold-button scaffold-button-direction"
      onmousedown="${buttonCallbackStrings.Right.down}"
      ontouchstart="${buttonCallbackStrings.Right.down}"
      onmouseup="${buttonCallbackStrings.Right.up}"
      ontouchend="${buttonCallbackStrings.Right.up}"
      >→</button>  
  </div>
</div>
`;

const controlsButtonsAb = `
<div class="scaffold-buttons">
  <div class="scaffold-flex-row-flex-end">
    <button class="scaffold-button scaffold-button-confirm"
      onmousedown="${buttonCallbackStrings.Confirm.down}"
      ontouchstart="${buttonCallbackStrings.Confirm.down}"
      onmouseup="${buttonCallbackStrings.Confirm.up}"
      ontouchend="${buttonCallbackStrings.Confirm.up}"
      >A</button>
  </div>
  <div class="scaffold-flex-row-center">
    <button class="scaffold-button scaffold-button-cancel"
      onmousedown="${buttonCallbackStrings.Cancel.down}"
      ontouchstart="${buttonCallbackStrings.Cancel.down}"
      onmouseup="${buttonCallbackStrings.Cancel.up}"
      ontouchend="${buttonCallbackStrings.Cancel.up}"
      >B</button>
  </div>
</div>
`;

const controlsButtonsAbShift = `
<div class="scaffold-buttons">
  <div class="scaffold-flex-row-center">

  </div>
  <div class="scaffold-flex-row-space-between">
    <button class="scaffold-button scaffold-button-shift"
      onmousedown="${buttonCallbackStrings.Shift.down}"
      ontouchstart="${buttonCallbackStrings.Shift.down}"
      onmouseup="${buttonCallbackStrings.Shift.up}"
      ontouchend="${buttonCallbackStrings.Shift.up}"
      >Y</button>
    <button class="scaffold-button scaffold-button-confirm"
      onmousedown="${buttonCallbackStrings.Confirm.down}"
      ontouchstart="${buttonCallbackStrings.Confirm.down}"
      onmouseup="${buttonCallbackStrings.Confirm.up}"
      ontouchend="${buttonCallbackStrings.Confirm.up}"
      >A</button>
  </div>
  <div class="scaffold-flex-row-center">
    <button class="scaffold-button scaffold-button-cancel"
      onmousedown="${buttonCallbackStrings.Cancel.down}"
      ontouchstart="${buttonCallbackStrings.Cancel.down}"
      onmouseup="${buttonCallbackStrings.Cancel.up}"
      ontouchend="${buttonCallbackStrings.Cancel.up}"
      >B</button>
  </div>
</div>
`;

// dpadLayout: normal, lr
// buttonsLayout: 'ab', 'abshift'
window.createOnScreenControls = (dpadLayout, buttonsLayout) => {
  const controlsDpad =
    dpadLayout === 'lr' ? controlsDpadLr : controlsDpadNormal;
  const controlsButtons =
    buttonsLayout === 'abshift' ? controlsButtonsAbShift : controlsButtonsAb;

  const scaffoldHtml = `
  <div class="scaffold-outer">
    <div id="scaffold-controls-bottom">
      <div class="scaffold-controls">
        ${controlsDpad}
        <div class="scaffold-vertical-spacer"></div>
      </div>
      <div class="scaffold-controls">${controlsButtons}
      <div class="scaffold-vertical-spacer"></div>
    </div>
  </div>
  `;
  const game = document.getElementById('controls');
  if (game) {
    game.innerHTML = scaffoldHtml;
  } else {
    console.error('Could not find "#controls" element to append scaffold.');
  }
}
