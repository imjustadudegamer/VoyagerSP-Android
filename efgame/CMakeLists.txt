cmake_minimum_required(VERSION 3.16)
project(efgame_sp CXX C)

# Elite Force single-player game modules (Raven STEF SP source) built as Android
# SHARED libraries — the M1.0 packaging the engine dlopen()s:
#   libefgame.so = game + cgame + icarus (+speedrun)  — exports GetGameAPI, vmMain, dllEntry
#   libefui.so   = ui                                 — exports GetUIAPI
# A version script exports ONLY those entry points and localizes everything else
# (so the 6 symbols game/ui share — Com_Printf etc — never collide, with each other
# or with the host engine). Mirrors retail efgamex86.dll / efuix86.dll.
#
# Build (armv7):
#   cmake -B build -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
#     -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-24
#   cmake --build build   -> build/libefgame.so, build/libefui.so

set(CMAKE_CXX_STANDARD 17)         # speedrun/ uses std::filesystem
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(AND ${CMAKE_SOURCE_DIR}/android)

# ---- shared interface include dirs + compat shim ----
set(EF_INCLUDES
  ${AND}                            # compat shim (windows.h/direct.h stand-ins) — FIRST
  ${SRC}/game ${SRC}/cgame ${SRC}/ui ${SRC}/icarus
  ${SRC}/qcommon ${SRC}/client ${SRC}/renderer
  ${SRC}/speedrun ${SRC}/speedrun/strafe_helper ${SRC}/speedrun/overbounce_prediction
)
set(EF_COPTS
  # Optimize the native game/cgame/ICARUS code. This module is built via a bare
  # `cmake --build` with no CMAKE_BUILD_TYPE, so without this the toolchain emits
  # -O0 and the per-frame cgame draw + 20Hz game sim run unoptimized on the main
  # thread -> the renderer front end is CPU-starved -> low fps / stutter on
  # weaker devices. -fno-strict-aliasing: idTech3/Raven code type-puns freely.
  -O2
  -fno-strict-aliasing
  -include ${AND}/ef_android_compat.h
  -ferror-limit=0
  -Wno-everything
  -fno-threadsafe-statics   # single-threaded modules; also dodges NDK r25 libc++abi
                            # cxa_guard ARM/Thumb-interworking link bug (syscall not STT_FUNC)
  -fcommon                  # MSVC/old-gcc merged tentative defs (e.g. `game_import_t gi;` in
                            # both g_main and gameinfo); modern clang is -fno-common by default
)

# ---- shared utility sources (compiled into BOTH modules, like the retail projects) ----
# q_math/q_shared/q_color etc. live in game/ but the ui module needs them too. Discover by
# pattern: the q_*.cpp shared core (NOT bg_lib = QVM libc).
file(GLOB EF_SHARED_SRC ${SRC}/game/q_math.cpp ${SRC}/game/q_shared.cpp ${SRC}/game/q_color.cpp)

# ===================== libefgame.so (game + cgame + icarus) =====================
file(GLOB_RECURSE EFGAME_SRC
  ${SRC}/game/*.cpp ${SRC}/cgame/*.cpp ${SRC}/icarus/*.cpp
  ${SRC}/speedrun/*.cpp ${SRC}/speedrun/*.c ${SRC}/speedrun/strafe_helper/*.c)
# gameinfo.cpp lives in ui/ but is a SHARED source (defines weaponData/ammoData tables);
# retail game.vcxproj compiles ..\ui\gameinfo.cpp too.
list(APPEND EFGAME_SRC ${SRC}/ui/gameinfo.cpp)
list(REMOVE_ITEM EFGAME_SRC ${SRC}/game/bg_lib.cpp)   # QVM freestanding libc — native uses platform libc
# The retail game project (game.vcxproj) compiles only these ICARUS files. Interface.cpp is a
# stub (real Interface_Init lives in game/Q3_Interface.cpp); Interpreter.cpp + Tokenizer.cpp are
# the offline BehavEd script compiler (in-game loads precompiled .ibi). Excluding them.
list(REMOVE_ITEM EFGAME_SRC
  ${SRC}/icarus/Interface.cpp ${SRC}/icarus/Interpreter.cpp ${SRC}/icarus/Tokenizer.cpp)

add_library(efgame SHARED ${EFGAME_SRC})
target_include_directories(efgame PRIVATE ${EF_INCLUDES})
target_compile_options(efgame PRIVATE ${EF_COPTS})
target_compile_definitions(efgame PRIVATE NDEBUG)
target_link_options(efgame PRIVATE
  -Wl,--version-script=${AND}/efgame.ver
  -Wl,--unresolved-symbols=ignore-all   # engine syscalls resolved at dlopen; libc/libm linked below
  # NDK r25 adds -Wl,--fatal-warnings by default; on armeabi-v7a the prebuilt
  # libc++abi cxa_guard.o emits a benign ARM/Thumb interworking warning that
  # then fails the link. Override (last-one-wins) so armv7 links cleanly.
  -Wl,--no-fatal-warnings)
target_link_libraries(efgame m)

# ===================== libefui.so (ui) =====================
file(GLOB EFUI_SRC ${SRC}/ui/*.cpp)
# Retail ui.vcxproj compiles ..\game\g_weaponLoad.cpp with _USRDLL;UI_EXPORTS so its
# WP_LoadWeaponParms() populates weaponData/ammoData on the UI side (loadout/weapon menus).
# Under _USRDLL the FX_*Think func-table (the #else / #ifndef _USRDLL branch) is excluded ->
# zero cgame deps; the only symbol it then needs is gameinfo's `gi` (see gameinfo.cpp guard).
list(APPEND EFUI_SRC ${SRC}/game/g_weaponLoad.cpp)
add_library(efui SHARED ${EFUI_SRC} ${EF_SHARED_SRC} ${EFUI_EXTRA_SRC})
target_include_directories(efui PRIVATE ${EF_INCLUDES})
target_compile_options(efui PRIVATE ${EF_COPTS})
# _USRDLL must be target-wide (not per-file) so the globbed gameinfo.cpp also takes the UI branch.
# Do NOT add _USRDLL to the efgame target (the game build keeps g_weaponLoad's cgame funcs[] table).
target_compile_definitions(efui PRIVATE NDEBUG _USRDLL UI_EXPORTS)
target_link_options(efui PRIVATE
  -Wl,--version-script=${AND}/efui.ver
  -Wl,--unresolved-symbols=ignore-all
  -Wl,--no-fatal-warnings)   # see efgame note above (NDK r25 armv7 libc++abi interworking)
target_link_libraries(efui m)
