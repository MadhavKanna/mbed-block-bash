# MBed Block Bash

## Description
MBed Block Bash is a multiplayer Tetris game designed to be played on Mbed-powered boards. It supports up to three simultaneous players, each controlling their own game using wireless controllers. The game utilizes the board's accelerometer, gyroscope, and user button for intuitive gameplay. A PC is used for display purposes only, rendering the game interface via Wi-Fi.

## Scope
- Wi-Fi server for game console (or Bluetooth with controllers as peripherals).
- Multiple wireless controllers capable of identifying themselves for individual game control.
- Display options: Laptop screen for display, with potential for direct connection to an LCD display on the console board.

## Technical Description
The project involves creating a multiplayer Tetris game running on Mbed-powered boards. The main components include the console board, peripheral controller boards, and a PC for display.

### Console
- Runs game logic for Tetris.
- Renders game image on display.
- Hosts a web server for remote display rendering.
- Handles wireless communication with controllers.

### Controllers
- Connect to the console over Wi-Fi.
- Control game actions using accelerometer, gyroscope, and user button.
- Automatically pair with the console when powered on.
- Support up to three controllers for simultaneous gameplay.

### Gameplay
- Each game iteration runs on a configurable time interval.
- Players control falling blocks using controller inputs.
- Game ends if a controller disconnects or if the console's user button is pressed.
- Display adjusts dynamically based on the number of connected controllers.

## Installation
- Clone the repository.
- Install necessary dependencies.
- Flash firmware onto Mbed boards.
- Connect controllers to the console over Wi-Fi.

## Usage
- Power on the console and controllers.
- Wait for controllers to pair with the console.
- Press the user button on the console to start the game.
- Use controller inputs to control falling blocks.
- Enjoy multiplayer Tetris action!

## Contributors
- Eric Pimentel Aguiar
- Kyle Yang
- Madhav Kanna Thenappan
- Victor Gasnikov

## License
This project is licensed under the [MIT License](\LICENSE).

