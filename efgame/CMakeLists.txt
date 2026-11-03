cmake_minimum_required(VERSION 3.16)
project(efgame_sp CXX C)

# Elite Force single-player game modules (Raven STEF SP source) built as SHARED
# libraries the engine dlopen()s:
#   libefgame.so = game + cgame + icarus (+speedrun)  — exports GetGameAPI, vmMain, dllEntry
#   libefui.so   = ui                                 — exports GetUIAPI
# A version script exports ONLY those entry points and localizes everything else
# (so the 6 symbols game/ui share — Com_Printf etc — never collide, with each other
# or with the host engine). Mirrors retail efgamex86.dll / efuix86.dll.
#
# TWO PLATFORMS, one tree (same EF_PLATFORM switch as EFAndroid-SP/app/jni):
#   EF_PLATFORM=android (the NDK toolchain) — the APK modules, armv7 / arm64.
#   EF_PLATFORM=linux   (the host compiler)  — the desktop modules, x86-64. Nothing
#       platform-specific changes here: the Raven sources are the same and the
#       ef_android_compat.h shim is a portable Win32/MSVC stand-in (it has always
#       said "on Android/Linux"), so only flags and the output dir differ.
#
# Build (armv7):
#   cmake -B build -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
#     -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-24
#   cmake --build build   -> build/libefgame.so, build/libefui.so
#
# Build (Linux), standalone:
#   cmake -B build-linux -G Ninja && cmake --build build-linux
#   -> build-linux/Release/libefgame.so, build-linux/Release/libefui.so
# The unified desktop build does not need this: EFAndroid-SP/app/jni/CMakeLists.txt
# add_subdirectory()s this project on Linux, so one configure+build produces the
# engine binary and both modules in the same Release/ directory.

# ---------------------------------------------------------------------------
# Platform switch — default from the toolchain, overridable with -DEF_PLATFORM=...
# ---------------------------------------------------------------------------
if(NOT DEFINED EF_PLATFORM)
  if(ANDROID)
    set(EF_PLATFORM "android")
  else()
    set(EF_PLATFORM "linux")
  endif()
endif()
if(NOT EF_PLATFORM MATCHES "^(android|linux)$")
  message(FATAL_ERROR "EF_PLATFORM must be 'android' or 'linux' — got '${EF_PLATFORM}'")
endif()
set(EF_PLATFORM "${EF_PLATFORM}" CACHE STRING "Target platform: android | linux" FORCE)
set_property(CACHE EF_PLATFORM PROPERTY STRINGS android linux)
message(STATUS "efgame: EF_PLATFORM = ${EF_PLATFORM}")

set(CMAKE_CXX_STANDARD 17)         # speedrun/ uses std::filesystem
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE_CURRENT_SOURCE_DIR, not CMAKE_SOURCE_DIR: identical when this project is
# configured on its own (the Android flow), and correct when the unified Linux
# build pulls it in with add_subdirectory() (where CMAKE_SOURCE_DIR is jni/).
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(AND ${CMAKE_CURRENT_SOURCE_DIR}/android)

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
  -fno-threadsafe-statics   # single-threaded modules; also dodges NDK r25 libc++abi
                            # cxa_guard ARM/Thumb-interworking link bug (syscall not STT_FUNC)
  -fcommon                  # MSVC/old-gcc merged tentative defs (e.g. `game_import_t gi;` in
                            # both g_main and gameinfo); modern clang is -fno-common by default
  # Strip the absolute host build path from __FILE__ / assert strings / debug info so
  # the developer's home directory never leaks into the shipped .so (or any crash log
  # a user sends back). -ffile-prefix-map covers debug info + __FILE__; -fmacro-prefix-map
  # covers __FILE__ used inside macros (assert). Maps this source root to a neutral token.
  -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src
  -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src
)

# Warning/error-limit flags are compiler-specific: -ferror-limit and -Wno-everything
# are clang (what the NDK and a clang host use); GCC rejects -ferror-limit outright
# and does not know -Wno-everything. -w is the GCC spelling of "-Wno-everything" for
# code we do not own. The Raven sources must stay warning-level clean, not edited.
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  list(APPEND EF_COPTS -ferror-limit=0 -Wno-everything)
else()
  list(APPEND EF_COPTS -w)
endif()

set(EF_LINK_OPTS
  -Wl,--unresolved-symbols=ignore-all   # engine symbols would resolve at dlopen, but the
                                        # modules have no engine imports at all: everything
                                        # goes through the gi/ui import tables. Kept so a
                                        # stray import fails at build time, not at load time.
)
# NOTE: the version script is per target (efgame.ver vs efui.ver) — appending it to the
# shared EF_LINK_OPTS above once linked libefui.so with efgame.ver, which silently
# localized GetUIAPI() and left the module with no exports at all.
if(EF_PLATFORM STREQUAL "android")
  list(APPEND EF_LINK_OPTS
    -Wl,-z,max-page-size=16384   # 16 KB page-size alignment (Android 15+); NDK r25 defaults to 4 KB
    # NDK r25 adds -Wl,--fatal-warnings by default; on armeabi-v7a the prebuilt
    # libc++abi cxa_guard.o emits a benign ARM/Thumb interworking warning that
    # then fails the link. Override (last-one-wins) so armv7 links cleanly.
    -Wl,--no-fatal-warnings)
endif()

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
# Exports ONLY GetGameAPI/vmMain/dllEntry; every other symbol is local (that is what
# keeps the six names game+ui share — Com_Printf, Com_Error, ... — from colliding with
# the engine or with libefui.so in the same process).
target_link_options(efgame PRIVATE -Wl,--version-script=${AND}/efgame.ver ${EF_LINK_OPTS})
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
# Exports ONLY GetUIAPI (see the efgame note above; efui.ver is a different script).
target_link_options(efui PRIVATE -Wl,--version-script=${AND}/efui.ver ${EF_LINK_OPTS})
target_link_libraries(efui m)

# ---------------------------------------------------------------------------
# Output location.
#   Android: unchanged — the .so lands at the build root, which is what the Gradle
#            flow greps up (`cp efgame/build64/lib*.so app/libs/arm64-v8a/`).
#   Linux:   <build>/Release/, i.e. beside the voyagersp binary, so the engine's
#            dlopen("libefgame.so") / dlopen("libefui.so") finds both with no
#            install step and no LD_LIBRARY_PATH juggling. True both standalone and
#            when the unified jni build pulls this project in (there CMAKE_BINARY_DIR
#            is the jni build dir, so Release/ is literally the same directory the
#            engine's RUNTIME_OUTPUT_DIRECTORY uses). Override with -DEF_GAME_OUTPUT_DIR.
# ---------------------------------------------------------------------------
if(EF_PLATFORM STREQUAL "linux")
  set(EF_GAME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Release" CACHE PATH
      "Directory for libefgame.so / libefui.so (Linux builds)")
  set_target_properties(efgame efui PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${EF_GAME_OUTPUT_DIR}")
  message(STATUS "efgame: modules -> ${EF_GAME_OUTPUT_DIR}")
endif()
