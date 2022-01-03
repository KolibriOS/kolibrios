# Hydra Castle Labyrinth

![HCL build status](https://api.travis-ci.org/ptitSeb/hydracastlelabyrinth.png "HCL build status")

This version of Hydra Castle Labyrinth is based on the 3DS port (see below for original notice)

This version use SDL (either 1.2 or 2.0) and build on Linux, OpenPandora, PocketCHIP and ODROID.

You'll need SDL and SDL_mixer to build, either version 1.2 or 2.0.
The SDL2 version use SDL Renderer and so is hardware accelerated.
The SDL1.2 version use plain bitmap (no OpenGL or GLES needed).

On Debian and friend, to prepare and build, you can do (if you never build anything before, start with `sudo apt install build-essential git`):
For SDL1.2
```
sudo apt install libsdl-dev libsdl-mixer1.2-dev cmake
cd ~
git clone https://github.com/ptitSeb/hydracastlelabyrinth.git
cd hydracastlelabyrinth
cmake .
make
```
For SDL2.0
```
sudo apt install libsdl2-2.0-dev libsdl2-mixer2.0-dev cmake
cd ~
git clone https://github.com/ptitSeb/hydracastlelabyrinth.git
cd hydracastlelabyrinth
cmake . -DUSE_SDL2=ON
make
```

To hear music, you can optionnaly use timidity, but it will play OGG track by default.
```
sudo apt install timidity
```
And launch the game with
```
./hcl
```
To start windowed 640x480 game. You can have fullscreen with `./hcl -f` or `./hcl -d` to have fullscreen at current desktop resolution.

![sreenshot on Pandora](screenshot.png "screenshot on Pandora")

# Web Version

You can play an Emscripten version directly on your browser here: https://ptitseb.github.io/hydracastlelabyrinth/

# Original Notice



**This work has been done by an anon from /hbg/ on 4chan.org/vg/ and not me!**




I've asked the original author about what license to use and he allowed me to use the GPLv2.
Therefore consider each file to be licensed under the GPLv2, even if there is no disclaimer inside of each file.
You will have gotten a copy of the license as part of the git and if not, get a copy from `https://www.gnu.org/licenses/gpl-2.0.html`.

Original author notes:
Source code for the fan-made port of Hydra Castle Labyrinth for 3DS

Anything related to the PSP and Wii are unfinished.

(Yes, it does look like a 3rd grader programmed this.)


The game's originally done by E.Hashimoto (a.k.a. Buster).
You can download some of his works under this [link](http://hp.vector.co.jp/authors/VA025956/).
