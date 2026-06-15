# efgame — EF1 single-player game module, building for Android

This tree compiles the **Star Trek: Voyager — Elite Force single-player game module**
(Raven STEF SP source, via the `Elite-Reinforce` fork) for Android. It builds as a **static
library** — it deliberately does **not** link: the engine half (`game_import_t`/syscall
implementations) is the next milestone. Getting this to compile is Milestone 0→1 of the SP port.

## Layout
- `src/` — the game module (`game/ cgame/ ui/ icarus/ speedrun/`) + interface headers
  (`qcommon/ client/ renderer/`). Copied from `../../Elite-Reinforce` and normalized for a
  case-sensitive, non-MSVC toolchain.
- `android/` — the compatibility layer (does the heavy lifting, keeps `src/` near-pristine):
  - `ef_android_compat.h` — force-included into every TU. Win32 typedefs, MSVC string
    intrinsics, calling-convention no-ops, `random()`→`ef_frandom` rename, and stdio-backed
    stand-ins for the ICARUS tokenizer's Win32 file API.
  - `windows.h`, `direct.h`, `conio.h` — stand-ins so the source's `#include <windows.h>` etc.
    resolve to the compat layer on Linux.
- `CMakeLists.txt` — static-lib target; forces the compat header; excludes `bg_lib.cpp`
  (the QVM freestanding libc — native builds use the platform libc).

## Build
```sh
NDK=$ANDROID_SDK_ROOT/ndk/25.1.8937393      # or wherever
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-24
cmake --build build
# -> build/libefgame_sp.a  (152 objects)
```

## Status
- **M1.0 packaging DONE — builds as two Android shared libraries for armv7:**
  - `build/libefgame.so` (~7.8 MB) — game+cgame+icarus(+speedrun) — exports **only**
    `GetGameAPI`, `vmMain`, `dllEntry`; **zero** genuine unresolved symbols.
  - `build/libefui.so` (~1.1 MB) — ui — exports **only** `GetUIAPI`; zero unresolved.
  - Both: NEEDED = libm/libc++_shared/libdl/libc only. The modules are self-contained and
    reach the engine purely through the passed-in function-pointer tables. Configure with
    `-DANDROID_STL=c++_shared` (see build cmd above). Version scripts in `android/*.ver`.
- **armv7 (armeabi-v7a): all 152 TUs compile.** (earlier M0 static-lib milestone)
- **arm64 (arm64-v8a): 245 `pointer→int` truncation errors in 24 files** — and nothing else.
  This is the only architecture-specific work left; the audit list is
  `build64/ptr_audit_files.txt`. Build armv7 first (it matches the shipping HM ABI and the
  `vm_armv7l` lineage); arm64 is a later hardening pass.

## What was changed vs upstream (Elite-Reinforce)
All edits are mechanical and catalogued in `../../SP_PORT_M0.md`:
- include case + `\`→`/` normalization (scripted);
- 3 `rep stosd` inline-asm blocks in `g_objectives.cpp` → C loops; `q_math.cpp` asm is
  `#else`'d out automatically on non-x86;
- 4 `std::map` typedefs: dropped the pre-standard explicit `std::allocator<T>`;
- `FX_AddSpawner`: added typed forwarder overloads (callers mixed direct-fn and `(void*)`);
- `ui_atoms.cpp`: removed mistaken `static` on 13 exported `UI_*` defs;
- `bg_infoItemList` made `extern` in its header; 2 `return false`→`NULL` in `BlockStream.cpp`;
- `sound_skipping.cpp`: C++20 `.contains` → `.count()`; build is C++17 for `std::filesystem`.

⚠️ License: the `src/` game code is under the Raven STEF mod EULA (not GPL). Fine to build/port
locally; shipping it inside a GPL APK is a separate distribution decision.
