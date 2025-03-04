# Chip8 Interpreter in C++
A Chip8 Interpreter running in terminal. Works only on Linux!
In an IDE terminal not all features may work.

## Usage
    ./Chip8Interpreter  [Clock (Hz)] /path/to/rom

Without specifying the clock it is set to 60Hz.
Some ROMs need a much higher or lower clock speed.

## Building
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

## Keypad

| Chip 8 Key | Keyboard Key |
| :--------: | :----------: |
| `1`        | `1`          |
| `2`        | `2`          |
| `3`        | `3`          |
| `4`        | `Q`          |
| `5`        | `W`          |
| `6`        | `E`          |
| `7`        | `A`          |
| `8`        | `S`          |
| `9`        | `D`          |
| `0`        | `X`          |
| `A`        | `Y`          |
| `B`        | `C`          |
| `C`        | `4`          |
| `D`        | `R`          |
| `E`        | `F`          |
| `F`        | `V`          |

