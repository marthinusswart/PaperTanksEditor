amiga-debug Visual Studio Code Extension

# Amiga-Debug Visual Studio Code Extension

Repository: [BartmanAbyss/vscode-amiga-debug](https://github.com/BartmanAbyss/vscode-amiga-debug)

## Overview

One-stop Visual Studio Code Extension to compile, debug, and profile Amiga C/C++ programs compiled by the bundled GCC 15.1 with the bundled WinUAE/FS-UAE and GDB.

This fully self-contained extension helps you quickly develop demos, intros, games, etc. for the Amiga 500, 1200, 4000. It supports C and C++ (no standard library). Advanced productivity features include debug overlay, frame profiler, graphics debugger, and size profiler.

**Debugger** | **Frame Profiler** | **Size Profiler** | **Graphics Debugger**

## Video

[v1.1 Features Demo (YouTube)](https://www.youtube.com/watch?v=gQ4tKisnr7Y)

## Quick Start

1. Install the extension from the Visual Studio Code Marketplace
2. (Recommended) Install the Microsoft C/C++ extension from the Extensions tab under Recommended
3. Create a new empty project folder with `File > Open Folder...`
4. From the command palette <kbd>Ctrl+Shift+P</kbd> select `Amiga: Init Project`
5. (Optional, but recommended)
   - Open settings <kbd>Ctrl+,</kbd> and under Extensions > Amiga C/C++ Compile, Debug & Profile, set Rom-paths: A500 to your Kickstart 1.3 ROM
   - Or: open `.vscode/launch.json` and set `"kickstart"` to your Kickstart 1.3 ROM
6. Hit <kbd>F5</kbd> to build and run a minimal sample project
7. If you prefer C++ instead of C, rename `main.c` to `main.cpp`

## Features

- No additional tools required. Everything is included (except Kickstart ROM). Ready to make your next Amiga 500 production!
- State-of-the-art code generation by GCC with Link-Time-Optimizations (LTO) for increased performance and smaller code size
- IntelliSense for C, C++, and assembly (VASM or GAS)
- Full source-level and assembly-level debugging with callstack, breakpoints, data breakpoints (watchpoints), watches, registers, memory view with GDB-enabled WinUAE
- Fully AmigaOS compatible via included `.h` files
- `INCBIN`, `INCBIN*CHIP` support
- Output to debug console from WinUAE
- WinUAE warp-launch (turbo-boot)
- WinUAE warp-mode control from your Amiga project (speed up precalc during development)
- WinUAE debug overlay, full control from your Amiga project
- Frame Profiler: function-level + DMA cycles profiling (A500, A1200 (PAL) only)
- Graphics Debugger: replay a captured frame cycle by cycle and watch your bitmaps change in real-time; Visualize all blitter operations, bitmap writes, copper list, custom registers (OCS only)
- Size Profiler: profile the size of your executable by functions, data and references
- Shrinkler executable cruncher with size profiling: profile the size of your compressed executable (right-click Amiga EXE files in the explorer)
- Disassembly: Show disassembly of ELF file incl. 68000 cycle count and correlation with source code (right-click Amiga ELF files in the explorer)
- Terminal window with access to the build tools
  modified Shrinkler 4.6
  Copyright 1999-2015 Aske Simon Christensen
  Modified 2020, Bartman/Abyss
  modified elf2hunk (source included)
  Copyright (c) 1995-2017, The AROS Development Team. All rights reserved.
  Modified 2018-2020, Bartman/Abyss
  GNU Make 4.2.1
  Copyright (C) 1988-2016 Free Software Foundation, Inc.
  License GPLv3+: GNU GPL version 3 or later http://gnu.org/licenses/gpl.html
  This is free software: you are free to change and redistribute it. There is NO WARRANTY, to the extent permitted by law.
  cd, EndCLI, run from Workbench 1.3
  unpacked exe2adf
  Copyright (c) 2015-2022 Bonefish/Reality. All rights reserved.
  modified VASM 1.9
  Copyright (c) 2002-2022 by Volker Barthelmann.
  see vasm-LICENSE
  Caveats
  sometimes when you're multiplying 2 WORDs together, gcc tries to use a (slow) 32-bit multiply. So if you have performance-critical multiplications, consider using the muluw and mulsw functions from gcc8_c_support.h
  Contributing
  For development, just install the latest node.js LTS, create a new directory, clone the repository git clone https://github.com/BartmanAbyss/vscode-amiga-debug.git, then install the dependencies with npm install. To build, open the directory in VS Code and hit F5. You can then test the extension without building a .vsix. To build a .vsix, npm install -g vsce (once), and then vsce package. For better testing, install the Mocha Test Explorer extension. The tests should then show up in your Testing tab.

Development (Windows)
Here are the command-lines used to compile the external tools (We're building with MinGW on WSL on Windows 10/11 to c:\amiga-mingw\opt). Replace the 16 in make -j16 with your number of CPU cores

Ubuntu 22.4 LTS from Microsoft Store
Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux
Enable-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform
wsl --install -d ubuntu
MinGW on WSL2 (Ubuntu 22.04)
sudo apt install build-essential flex bison expect dejagnu texinfo mingw-w64
sudo update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

# statically link pthread (for GDB) - see https://stackoverflow.com/a/72903594

sudo mv /usr/x86_64-w64-mingw32/lib/libwinpthread.dll.a /usr/x86_64-w64-mingw32/lib/libwinpthread.dll.a.bak
sudo mv /usr/x86_64-w64-mingw32/lib/libpthread.dll.a /usr/x86_64-w64-mingw32/lib/libpthread.dll.a.bak

# expat for gdb's tartget.xml support

wget https://github.com/libexpat/libexpat/releases/download/R_2_6_0/expat-2.6.0.tar.gz
tar xzf expat-2.6.0.tar.gz
cd expat-2.6.0
./configure --host=x86_64-w64-mingw32 --prefix=/usr/x86_64-w64-mingw32/
make -j16 # ignore errors
sudo make install # ignore errors
sudo mv /usr/x86_64-w64-mingw32/lib/libexpat.dll.a /usr/x86_64-w64-mingw32/lib/libexpat.dll.a.bak # statically link expat
sudo mv /usr/x86_64-w64-mingw32/lib/libexpat.la libexpat.la.bak
Binutils+GDB
git clone https://github.com/BartmanAbyss/binutils-gdb.git
cd binutils-gdb
bash ./contrib/download_prerequisites
cd ..
mkdir build-binutils-gdb
cd build-binutils-gdb
LDFLAGS="-static -static-libgcc -static-libstdc++" ../binutils-gdb/configure --prefix=/mnt/c/amiga-mingw/opt --target=m68k-amiga-elf --disable-werror -enable-static --disable-shared --disable-interprocess-agent --disable-libcc --host=x86_64-w64-mingw32 --with-expat
make -j16
make install
GCC
wget https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-15.1.0/gcc-15.1.0.tar.xz
tar -xf gcc-15.1.0.tar.xz
cd gcc-15.1.0
patch -p1 < ../gcc-barto.patch
bash ./contrib/download_prerequisites
cd ..
mkdir -p build-gcc-15.1.0
cd build-gcc-15.1.0
LDFLAGS="-static -static-libgcc -static-libstdc++" ../gcc-15.1.0/configure \
 --disable-clocale \
 --disable-gcov \
 --disable-libada \
 --disable-libgomp \
 --disable-libsanitizer \
 --disable-libssp \
 --disable-libvtv \
 --disable-multilib \
 --disable-threads \
 --disable-nls \
 --enable-languages=c,c++ \
 --enable-lto \
 --enable-static \
 --prefix=/mnt/c/amiga-mingw/opt \
 --target=m68k-amiga-elf \
 --host=x86_64-w64-mingw32
make all-gcc -j16

# at this point, you're getting an error about not finding gcc-cross. This is okay.

sed 's/selftest # srcextra/# selftest srcextra/' gcc/Makefile >gcc/Makefile.tmp
mv gcc/Makefile.tmp gcc/Makefile
gcc/gcc-cross.exe -dumpspecs >gcc/specs

# continue the build

make all-gcc -j16
make install-gcc
elf2hunk
https://github.com/BartmanAbyss/elf2hunk

Cleaning up unnecessary files and stripping EXE files of debug information to reduce size
rm -r /mnt/c/amiga-mingw/opt/include
rm -r /mnt/c/amiga-mingw/opt/share
find /mnt/c/amiga-mingw/opt -name _.exe | xargs strip
Development (Linux/MacOS)
Have a look at the CI scripts here https://github.com/BartmanAbyss/vscode-amiga-debug/tree/master/ci
Internal Development
Debugging
amigaDebug.ts: set DEBUG to TRUE to enable GDB/execution traces
profile_editor_provider.ts: set DEBUG to TRUE to enable preact-devtools
preact-devtools
git clone https://github.com/preactjs/preact-devtools.git
npm install
npm run build:inline
<copy dist/inline/_ to preact-devtools>
Updating node_modules
npm install -g npm-check
npm-check -u
WinUAE builds with Visual Studio 2022.

Create new GCC patch
diff -ruN gcc-15.1.0 gcc-15.1.0-barto > gcc-barto.patch
Known Issues/TODOs
Documentation
TODO: better documentation
Profiler
lines of functions seem to be off (see template/main.c: function main)
TODO: drag across flame-graph to measure durations
TODO: multi-frame profiling: allow user to select number of frames
TODO: code lenses: update when display unit changes, when frame changes
stack sizes > 1024 cause UnwindTable to fail parsing (#35)
TODO: new DMArecord fields (CIA)
Savestate Debugger
TODO: kill winuae/gdb when quitting vscode
(internal) status of tested savestates
desertdream-dots.uss: ok
interference-stars.uss: overdraw not correct
gods.uss: blitrects' height not correct due to planar layout
shadesbeat.uss: not showing any bitplanes due to not setting them in copper. TODO: get bitplanes from custom registers
brianthelion-rotozoom.uss: some blitrects missing
rinkadinkredux-end.uss: hover over small bits at the right, causes error with 0-sized canvas
rinkadinkredux-rotozoom.uss: no end detected for multiple blits (frame 2)
Assembly
TODO: parser needs to check for comments
TODO: more...
Objdump
TODO: click/follow any addresses
WinUAE
execution for A4000 model is very flaky (since 1.2.1)
TODO: fill memory with $DEAD on startup to better find uninitialized memory bugs
TODO: debugger: detect more exceptions in a better way (not just setting a single breakpoint at every exception vector)
Debugger
memory, disassembly: use VSCode built-in requests; memory: can't get memoryReference to work on WATCH items
disassembly chokes a bit on newlines
data breakpoints read/read-write (there doesn't seem to be an UI for this in VSCode)
store assembly breakpoints in one "virtual" file in breakpointMap (how?!)
vscode.debug.activeDebugSession is undefined when program is stopped on entry
not getting handleThreadSelected(), thread ID now set in class
sometimes Pause/Resume button doesn't correctly switch to "Pause" icon while amiga program is running
step out of kickstart: set fake breakpoint at 0xfffffff, WinUAE should enter TRACE_RANGE_PC mode (TODO: tighten range around loaded program), but keeps breaking later
in disassembly view, skipping subroutines with Step Over may not work in inlined functions (limitation of GDB)
Kickstart
TODO: support multiple vector tables per library (for FPU/non-FPU mathieeesingbas)
TODO: stack unwinding for kickstart (maybe not necessary)
Gfx Debugger
blitter doesn't get pointers if not explicitly written by CPU (e.g. reusing pointers after blit)
TODO: correctly show blits over 2 frames (second frame is missing blitbox)
TODO: show source blitter-rects
TODO: show 2 resources
TODO: tooltips for blitter-rects?
Denise: TODO: glitches, blitrects, overdraw, ECS/AGA scrolling, AGA sprites, multi-frame
Denise: turrican2-level1.uss, desertdream-dots.uss: sprites 2 pixels too far left
TODO: AGA colors (256 colors and 24bit)
