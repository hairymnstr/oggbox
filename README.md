OggBox
======

Author: nathan@nathandumont.com
License: C code LGPL V3, schematics CC-BY-SA 3.0

The OggBox is an open hardware music player.  It supports OggVorbis and WAV files
using a dedicated codec chip from VLSI.  The source code for the main processor
(an ARM Cortex) is written in C and can be found under src/firmware.  You can
compile it with a toolchain built with
https://github.com/esden/summon-arm-toolchain and can be flashed to an STM32 with
the http://code.google.com/p/stm32flash/ executable.  You may need to adjust the
Makefile as the install target uses a specific device on my computer so that it
doesn't try and access any other FTDI devices attached to my PC.

The schematics are in KiCAD format and are under src/sch there's a PDF of the
schematic frozen at the Rev A board production, gerber files for this are in the
src/sch/gerber folder at the moment.  You may need my KiCAD libraries to edit
these files, they're on GitHub too https://github.com/hairymnstr/ndkicadlibrary

Wiki
----

Wiki and blog posts are linked from http://oggbox.nathandumont.com

