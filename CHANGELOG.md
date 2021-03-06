
Version changelog
=================


Version 0.3.0
-------------

General

- Changed GPL v3 license by GPL v2.
- Added a lot of documentation about GB hardware. [GB]
- Controls can be configured.
- Support for game controllers.
- Rumble supported. In my joystick it doesn't work in Windows 7, but it works in Linux Mint 16...
- Added GB Camera viewer. [GB]
- Fixed sprites showing as hidden in the GB sprite viewer when they are in the bottom 8 rows of the screen (in 8x8 mode). [GB]
- Code reorganized.
- Update memory and I/O viewers pressing F7 in dissasembler. [GB/GBA]
- MBC3 timer data is now appended to the .sav file like other emulators do to make them compatible.

GB emulation

- GB core rewritten.
- Timer emulated perfectly in DMG. In GBC/GBA/GBA SP there are some cases that can't be emulated. (?)
- HDMA/GDMA timings corrected.
- Fixed KEY1 register when doing a hard reset.
- Joypad interrupt fixed.
- HALT instruction emulated correctly.
- GB Camera emulation greatly improved. Also, webcam can be selected with the ini configuration file (base exposure time can also be changed).

Version 0.2.0
-------------

General

- Complete new GUI using SDL2 (cross-plataform).
- Drag and drop ROMs supported.
- Create screenshot and bios folders if they don't exist.
- Breakpoints in disassembler. [GB/GBA]
- CPU registers can be changed. [GB/GBA]
- In GBA mode, the debugger steps to the next instruction even in halt mode (executing as clocks as needed, one frame max). In GB mode not.
- Memory can be edited in memory viewers. ASCII characters shown. [GB/GBA]
- Disassembly shows "true" or "false" to know if the next instruction will be executed (if it is conditional). [GB/GBA]
- Disassembly shows little arrows in branches to know where they go (up/down). [GB/GBA]
- Some fixes by looking at the commits of a fork of GiiBiiAdvance by wowzaman12 (and lioncash). Thanks!
- Better sprite viewers. [GB/GBA]
- Debugger windows can dump maps, sprites, palettes, tilesets... to png files.
- Better map and tile viewers. [GB]

GBA emulation

- Barrel shifter fixes (thanks, Normmat!).
- Undefined instructions in THUMB emulated (except that ones using r15 as operand).
- ldm/stm with writeback with base included in rlist hopefully fixed.
- Fixed DMA sound IRQ.
- SOUNDCNT_H can now be writen when sound is off.
- Special effects (blending, fade white or black) fixed.
- Transparent sprites are always transparent even when blending is disabled by the windows. Tested in hardware.
- GREENSWAP (04000002h) I/O register emulated.
- Disassembly improved.
- Disassembly shows I/O register names that are accesed by ldr/str instructions.
- Fixed a bug where some ALU instructions in arm mode would be executed with 0 clocks needed.

GB emulation

- DAA fixed (using a table extracted by testing all combinations on hardware).
- 'ld hl,sp+n' and 'add sp,n' flags fixed.
- CPU emulation rewriten. A lot more accurate, but SLOWER. It must be modified...
- '4x4 World Trophy (Europe) (En,Fr,De,Es,It)' works.
- Disassembly improved.
- Disassembly shows I/O register names that are accesed by memory instructions.
- Added targets for compiling with GB Camera.
- GB Camera emulation improved a lot since GiiBii. Contrast, enable/disable dithering, exposure time... There is only one game that use it so it's emulated for that game. Other software will probably fail (there are too many unknown things).


Version 0.1.0
-------------

- First release.
- Win32 GUI.
- GB, GBC, SGB and GBA emulation. GBA compatibility is still a bit low.
- Sound problems in GBA mode.
- Noise channel not correctly emulated (GB and GBA).
