# JFK Mario World
A remake/fangame of Super Mario World in C++/C. Sort of an engine for making Mario games that need to be based off of Super Mario World.

The code hosted here contains no copyrighted assets or any stolen code. Most things have been reverse engineered and recoded completely from the original game, while also trying to keep compatibility with custom resources made for the original game.

## Features
* Fully working engine with tons of features.
* Physics reverse engineered and recoded from the original game to be as close as possible to the original (except collision, which was recoded entirely).
* Compatibility with custom resources made for the original Super Mario World, except levels made with Lunar Magic.
* Unfinished, but working ASM 65c18 processor/emulator for running ASM scripts.
* Coding similar to making resources for the original game, except with remapped addresses and different logic to work with.
* Lua Support for scripting levels and sprites.
* Selling Feature: Multiplayer. Play the game with other people! However, this is currently an unstable and buggy feature.
* A world editor for making levels for the game similar to Lunar Magic, named Solar Energy
* Almost no limitations, 128 sprites on screen at once without any FPS drops.
* 4bpp/2bpp GFX rendering, OAM with more features like scaling, rotation, size, all palette rows.

## How can I help?
If you know how to code, submit fixes. If you like to test stuff, make levels and test the craziest stuff you can think of and try to find bugs, then report them in issues. Everything helps! We especially need help on making the game physics accurate. Also if a multiplayer server is up, help us test the netcode by just playing the game.

## Requirements / Dependencies
* SDL2
* SDL_mixer
* SDL_image
* SFML
* Lua 5.3
* SNES_SPC (Included with this, you need to build a .lib using the CMakeLists in there)

## Contact
Discord: https://discord.gg/cejeea8