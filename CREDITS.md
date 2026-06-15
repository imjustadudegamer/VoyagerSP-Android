# Credits & Acknowledgements

This Android single-player port stands on a large body of prior work. Every component below is
gratefully credited to its authors. Nothing here is claimed as original except the Android port
glue, the JNI/engine bridge, and the handheld-specific fixes noted at the bottom.

## The game

- **Star Trek: Voyager — Elite Force** — developed by **Raven Software**, published by **Activision**,
  under license from **Paramount**. All Elite Force trademarks, characters, and assets are the property
  of their respective owners. No retail assets are distributed in this repository.

## Single-player game module (`efgame/`)

- Based on the **official Elite Force Single-Player mod source SDK**, released by **Raven Software** for
  community modding. Public mirror: <https://github.com/UberGames/SP-Mod-Source-Code>.
- Licensed under the **STEF Game Source License** (Raven/Activision) — see that license; it is **not**
  GPL. The game module retains this license; only the Android port modifications are added on top.
- Thanks to the **"The Dark Project" / Voyager: Insurrection** mod team, whose advocacy led Raven to
  release the SP source SDK.

## Engine (`EFAndroid-SP/app/jni/efcode`, `engine-sp/`)

idTech3 lineage, all **GPLv2**:

- **Quake III Arena** — id Software. <https://github.com/id-Software/Quake-III-Arena>
- **ioquake3** — the ioquake3 contributors. <https://github.com/ioquake/ioq3>
- **lilium-voyager** — Zack Middleton — the ioquake3-based Elite Force engine port that this engine
  half builds on. <https://github.com/clover-moe/lilium-voyager>
- **Quake3e** — Eugene C. (ec-) — source of the Vulkan renderer (`renderervk`) and the AArch64/ARMv7
  VM JIT (`vm_aarch64.c`, `vm_armv7l.c`). <https://github.com/ec-/Quake3e>
- **VoyagerNX** — faithvoid — the Nintendo Switch port that proved the ARM + console-controls path this
  Android lineage started from. <https://github.com/faithvoid/VoyagerNX>

## Bundled libraries

- **SDL2** — Sam Lantinga and the SDL contributors (zlib license). <https://libsdl.org>
- **FFmpeg** — the FFmpeg team (LGPL/GPL) — used for Bink/RoQ cutscene video decoding.
  <https://ffmpeg.org>
- **libmad** 0.15.1b — Underbit Technologies (GPLv2) — MP3 audio decoding.
- **zlib** — Jean-loup Gailly and Mark Adler (zlib license).
- **libjpeg** (jpeg-8c) — the Independent JPEG Group.
- **LCC** — Christopher Fraser and David Hanson — the retargetable C compiler used to build QVM bytecode
  (`efcode/tools/lcc`).

## References consulted (not bundled as code)

- **OpenJK** — the JACoders team — reference for 64-bit idTech3 savegame/ICARUS behavior during the
  arm64 LP64 port. <https://github.com/JACoders/OpenJK>

## Not included (proprietary — supply your own)

- **Bink Video** — RAD Game Tools / Epic (`binkw32.dll`, `binkw32` runtime) — not redistributed.
- **Retail Elite Force game data** (`*.pk3`, retail `*.exe`/`*.dll`) — not redistributed; you must
  provide your own legally-owned copies.

## This port

- The Android single-player port and handheld integration — JNI engine bridge, Vulkan renderer fixes,
  touch and gamepad controls, 4:3/16:9 aspect handling, save-menu thumbnails, the ILP32↔LP64
  (armv7/arm64) savegame transcoder, and assorted bridge/cutscene/audio fixes — were contributed by the
  project's authors and contributors.

---

If any attribution here is incomplete or incorrect, please open an issue — it will be fixed promptly.
