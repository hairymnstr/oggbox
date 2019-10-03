OggBox
======

Author: nathan@nathandumont.com
License: 
* C code directly contained in this repository BSD-2-clause
* 3rd party libraries (FreeRTOS etc) see their own license terms
* schematic and CAD files CC0 1.0

The OggBox is an open hardware music player.  It supports OggVorbis and WAV files
using a dedicated codec chip from VLSI.  The source code for the main processor
(an ARM Cortex) is written in C and can be found under src/firmware.

Originally it was developed using the toolchain built with
<https://github.com/esden/summon-arm-toolchain> but can be built with newer
toolchains that provide arm-none-eabi-gcc and an appropriate newlib

    sudo apt install gcc-arm-none-eabi

will install a suitable toolchain on Ubuntu 18.04.

Flashing the device can be done via a suitable SWD probe and OpenOCD, or using
the FTDI header and <https://sourceforge.net/p/stm32flash/wiki/Home/>.  You may 
need to adjust the Makefile as the install target uses a specific device on my
computer so that it doesn't try and access any other FTDI devices attached to my PC.

The schematics have recently been ported to KiCAD v5.2 format and are under /sch
the libraries required are pulled in as local to the project as git submodules.

What documentation exists is linked from <http://oggbox.nathandumont.com>

