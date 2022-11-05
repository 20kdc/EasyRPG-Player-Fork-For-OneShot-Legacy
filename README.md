# NOTE:

## Disregard recent commits, these are *just* README.md fixes right now.

## This was modified by me (20kdc) in order to run the game 'OneShot' (freeware version, not the remake).
## As such, it contains much hackery in order to emulate hackery used by that version of OneShot.

## Furthermore, note that I'm unaffiliated with the developers of OneShot and the developers of EasyRPG-Player.

## The responsibility for the various bits and pieces marked "OneShot-Specific Hackery" is mine unless contributed externally,
## but is not that of either the EasyRPG-Player developers or the OneShot developers.

## The original at https://github.com/EasyRPG/Player should always be used unless dealing with this specific game,
## and really, this fork should only be used if you can't find any other alternative for a specific platform.

## Finally, I reiterate that this is for the original freeware OneShot, not the remake on Steam.

The code added for this may be considered a derivative of https://github.com/elizagamedev/oneshot-legacy , the license to which is:

    The MIT License
    
    Copyright (c) 2015 Mathew Velasquez (http://mathew.link/)
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.


# EasyRPG Player

EasyRPG Player is a game interpreter to play RPG Maker 2000, 2003 and EasyRPG
games. It uses the LCF parser library (liblcf) to read RPG Maker game data.

EasyRPG Player is part of the EasyRPG Project. More information is
available at the project website: https://easyrpg.org/


## Documentation

Documentation is available at the documentation wiki: https://wiki.easyrpg.org


## Requirements

### minimal / required

- liblcf for RPG Maker data reading. https://github.com/EasyRPG/liblcf
- SDL2 for screen backend support.
- Pixman for low level pixel manipulation.
- libpng for PNG image support.
- zlib for XYZ image support.

### extended / recommended

- FreeType2 for external font support (+ HarfBuzz for Unicode text shaping)
- mpg123 for better MP3 audio support
- WildMIDI for better MIDI audio support
- Ogg+Vorbis/Tremor for OGG audio support
- libsndfile for better WAVE audio support
- SpeexDSP for proper audio resampling
- SDL2_mixer for audio mixing. Used as a fallback when none of the provided
  audio libraries support the format. Due to API limitations not all audio
  effects are possible through SDL2_mixer audio.

SDL and SDL_mixer 1.2 are still supported, but deprecated.

## Daily builds

Up to date binaries for assorted platforms are available at our continous
integration service:

https://ci.easyrpg.org/view/Player/


## Source code

EasyRPG Player development is hosted by GitHub, project files are available
in this git repository:

https://github.com/EasyRPG/Player

Released versions are also available at our Download Archive:

https://easyrpg.org/downloads/player/


## Building

### Autotools Makefile method:

Building requirements:

- pkg-config
- GNU make

Step-by-step instructions:

    tar xf easyrpg-player-0.5.1.tar.xz # unpack the tarball
    cd easyrpg-player-0.5.1            # enter in the package directory
    ./configure                        # find libraries, set options
    make                               # compile the executable

Additional building requirements when using the source tree (git):

- autoconf >= 2.69
- automake >= 1.11.4
- libtool

To generate the "configure" script, run before following the above section:

    autoreconf -i

Read more detailed instructions at:

* https://wiki.easyrpg.org/development/compiling/player/autotools


### CMake method:

Building requirements:

- pkg-config
- CMake

Step-by-step instructions:

    tar xf easyrpg-player-0.5.1.tar.xz # unpack the tarball
    cd easyrpg-player-0.5.1            # enter in the package directory
    cmake .                            # generate Makefile
    make                               # compile the executable

Read more detailed instructions at:

* https://wiki.easyrpg.org/development/compiling/player/cmake


### Visual Studio method:

Building requirements:

- Visual Studio 2015 Update 2 or newer

Compile the dependencies in a Visual Studio command prompt:

    git clone https://github.com/EasyRPG/buildscripts
    cd buildscripts/windows
    powershell .\setup.ps1             # requires policy "RemoteSigned"
    build v140                         # compile all dependencies

Create an environment variable EASYDEV_MSVC pointing to buildscripts/windows/build

Compile the Player:

- liblcf is compiled as part of the Player. Extract/Clone liblcf in the ''lib'' directory
- Open ''builds/vs2015/Player.sln'' in Visual Studio
- The executable is created in the ''bin'' directory

Read detailed instructions at:

* https://wiki.easyrpg.org/development/compiling/player/windows/visual-studio


## Running EasyRPG Player

Run the `easyrpg-player` executable from a RPG Maker 2000 or 2003 game
project folder (same place as `RPG_RT.exe`).

## License

EasyRPG Player is free software available under the GPLv3 license. See the file
COPYING for license conditions.

### 3rd party software

EasyRPG Player makes use of the following 3rd party software:

* FMMidi YM2608 FM synthesizer emulator - Copyright (c) 2003-2006 yuno
  (Yoshio Uno), available under the (3-clause) BSD license
