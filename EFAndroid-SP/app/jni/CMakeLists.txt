cmake_minimum_required(VERSION 3.16...3.31)

project(GAME C CXX)

# ---------------------------------------------------------------------------
# THE PLATFORM SWITCH — one tree, two targets (Vulkan only on both)
#
#   EF_PLATFORM = android  (default under the NDK toolchain) -> libmain.so, into the APK
#   EF_PLATFORM = linux    (default everywhere else)         -> ./voyagersp, a desktop binary
#
# Everything above the substrate is shared and has no fork: the engine in
# efcode/, the SP bridge in sp/, the game modules in ../../efgame and the
# Vulkan renderer (renderervk, Quake3e) are compiled verbatim on both targets.
# Only the substrate changes:
#
#   * SDL        — vendored SDL2 (jni/SDL) on Android; the system SDL2 on Linux
#   * entry      — SDL_main (called by SDLActivity) on Android; plain main() on Linux
#   * data root  — the app's OBB directory on Android; the directory holding the
#                  binary (or +set fs_basepath) on Linux
#   * shaders    — the SPIR-V blob is compiled in on both (no runtime GLSL)
#
# The design mirrors Provenance EF2's EF2_PLATFORM switch, which is where the
# one-tree/two-target layout, the preset names and the Release/ output shape
# come from.
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
message(STATUS "EF_PLATFORM = ${EF_PLATFORM} (renderer: Vulkan)")

# Align shared-lib segments to 16 KB so the app is compatible with Android 15+
# 16 KB page-size devices (NDK 25 defaults to 4 KB). Android-only: on the
# desktop the page size is the host's own.
if(EF_PLATFORM STREQUAL "android")
  add_link_options(-Wl,-z,max-page-size=16384)
endif()

# Strip the absolute host build path from __FILE__ / assert strings / debug info so
# the developer's home directory never leaks into libmain.so (or any log a user sends
# back). Applies to every target defined below (SDL + engine + bridge). -ffile-prefix-map
# covers debug info + __FILE__; -fmacro-prefix-map covers __FILE__ inside macros (assert).
add_compile_options(
  -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src
  -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src)

# ---- substrate: SDL ----
if(EF_PLATFORM STREQUAL "android")
  # Vendored SDL2 sources (2.30.9) — the APK ships it as libSDL2.so.
  add_subdirectory(SDL)
else()
  # System SDL2 (>= 2.0.22 for SDL_Vulkan_CreateSurface). No vendored copy is
  # built on Linux: the distro package is the same library, already patched.
  find_package(SDL2 2.0.22 REQUIRED)
  message(STATUS "SDL2 (system): ${SDL2_VERSION}")
endif()

# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

# ---- game modules: built into the same tree on the desktop only ----
# The engine dlopen()s libefgame.so / libefui.so by bare name and both are built from
# ../../../../efgame. On Android that project is configured separately (the Gradle flow
# builds it per ABI and drops the .so into app/libs/<abi>/, which is what the APK ships),
# so adding it here would duplicate that step and fight the packaging. On Linux there is
# no Gradle: pulling it in is what makes one configure+build produce the binary AND its
# two modules in the same Release/ directory.
if(EF_PLATFORM STREQUAL "linux")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../efgame
                   ${CMAKE_BINARY_DIR}/efgame)
endif()

# Your game and its CMakeLists.txt are in a subfolder named "src"
add_subdirectory(src)
