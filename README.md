# Chip8 Interpreter in C++
A Chip8 Interpreter running in terminal. Works only on Linux!
In an IDE terminal not all features may work.

## Building
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

## Usage
    ./Chip8Interpreter  [cycle time (ms)] [instructions per frame] /path/to/rom

- Cycle time: Time per cycle. By default set to 16ms
- Instructions per frame: The amount of instructions which are run in one cycle. By default set to 11
- Path: A path to a ROM has to be specified

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
| `A`        | `Z`          |
| `B`        | `C`          |
| `C`        | `4`          |
| `D`        | `R`          |
| `E`        | `F`          |
| `F`        | `V`          |

