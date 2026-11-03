cmake_minimum_required(VERSION 3.6)
project(efmain C)

# ---------------------------------------------------------------------------
# Elite Force (lilium-voyager) engine -> libmain.so for Android (SDL2 harness)
# Single-binary build: interpreted VM (NO_VM_COMPILED), renderervk (Vulkan) statically
# linked (no USE_RENDERER_DLOPEN; the dead GL renderers were stripped), SDL audio, no curl/ogg/opus/mp3.
# ---------------------------------------------------------------------------

set(EF ${CMAKE_CURRENT_SOURCE_DIR}/../efcode)

# ---- engine source groups (mirrors Makefile.nx SOURCEFILES, minus _nx/codecs) ----
set(CLIENT_SRC
  ${EF}/client/cl_cgame.c   ${EF}/client/cl_cin.c     ${EF}/client/cl_console.c
  ${EF}/client/cl_input.c   ${EF}/client/cl_keys.c    ${EF}/client/cl_main.c
  ${EF}/client/cl_net_chan.c ${EF}/client/cl_parse.c  ${EF}/client/cl_scrn.c
  ${EF}/client/cl_ui.c      ${EF}/client/cl_avi.c
  ${EF}/client/snd_adpcm.c  ${EF}/client/snd_dma.c    ${EF}/client/snd_mem.c
  ${EF}/client/snd_mix.c    ${EF}/client/snd_wavelet.c
  ${EF}/client/snd_main.c   ${EF}/client/snd_codec.c  ${EF}/client/snd_codec_wav.c
  ${EF}/client/snd_codec_mp3.c
  ${EF}/client/qal.c        ${EF}/client/snd_openal.c)

# libmad (MP3 decoder) — EF1 SP voice-over ships as MP3; without this the engine
# only registers the WAV codec, every VO loads as defaultSound, and dialog is silent.
# snd_codec_mp3.c is libmad-backed (USE_INTERNAL_MP3). FPM_64BIT = portable pure-C
# fixed-point mode (no inline asm; builds on armv7 + arm64). mad.h is under include/.
set(LIBMAD_SRC
  ${EF}/libmad-0.15.1b/bit.c      ${EF}/libmad-0.15.1b/decoder.c
  ${EF}/libmad-0.15.1b/fixed.c    ${EF}/libmad-0.15.1b/frame.c
  ${EF}/libmad-0.15.1b/huffman.c  ${EF}/libmad-0.15.1b/layer12.c
  ${EF}/libmad-0.15.1b/layer3.c   ${EF}/libmad-0.15.1b/stream.c
  ${EF}/libmad-0.15.1b/synth.c    ${EF}/libmad-0.15.1b/timer.c
  ${EF}/libmad-0.15.1b/version.c)

set(QCOMMON_SRC
  ${EF}/qcommon/cm_load.c   ${EF}/qcommon/cm_patch.c  ${EF}/qcommon/cm_polylib.c
  ${EF}/qcommon/cm_test.c   ${EF}/qcommon/cm_trace.c
  ${EF}/qcommon/cmd.c       ${EF}/qcommon/common.c    ${EF}/qcommon/cvar.c
  ${EF}/qcommon/files.c     ${EF}/qcommon/md4.c       ${EF}/qcommon/md5.c
  ${EF}/qcommon/msg.c       ${EF}/qcommon/net_chan.c  ${EF}/qcommon/net_ip.c
  ${EF}/qcommon/huffman.c   ${EF}/qcommon/q_math.c    ${EF}/qcommon/q_shared.c
  ${EF}/qcommon/unzip.c     ${EF}/qcommon/ioapi.c     ${EF}/qcommon/puff.c
  ${EF}/qcommon/vm.c        ${EF}/qcommon/vm_interpreted.c)

# VM execution: armv7 has the proven SUSE JIT (vm_armv7l.c); arm64 has the
# Quake3e-derived JIT (vm_aarch64.c). Anything else -> interpreter.
if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
  list(APPEND QCOMMON_SRC ${EF}/qcommon/vm_armv7l.c)
  set(EF_VM_JIT TRUE)
elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
  list(APPEND QCOMMON_SRC ${EF}/qcommon/vm_aarch64.c)
  set(EF_VM_JIT TRUE)
else()
  set(EF_VM_JIT FALSE)
endif()

set(SERVER_SRC
  ${EF}/server/sv_bot.c     ${EF}/server/sv_ccmds.c   ${EF}/server/sv_client.c
  ${EF}/server/sv_game.c    ${EF}/server/sv_init.c    ${EF}/server/sv_main.c
  ${EF}/server/sv_net_chan.c ${EF}/server/sv_snapshot.c ${EF}/server/sv_world.c)

file(GLOB BOTLIB_SRC ${EF}/botlib/*.c)

set(SDL_SRC
  ${EF}/sdl/sdl_input.c ${EF}/sdl/sdl_snd.c ${EF}/sdl/sdl_gamma.c ${EF}/sdl/sdl_glimp.c)

set(SYS_SRC
  ${EF}/sys/con_log.c ${EF}/sys/con_passive.c ${EF}/sys/sys_autoupdater.c ${EF}/sys/sys_unix.c)
# sys_main.c compiled separately so we can rename main -> SDL_main
set(SYS_MAIN ${EF}/sys/sys_main.c)

# ---- x86-64 host: the SSE inline-asm helpers ----
# q_shared.h's idx64 branch maps Q_ftol/Q_SnapVector/Q_VMftol to qftolsse/qsnapvectorsse/
# qvmftolsse (round-to-nearest, ELITEFORCE flavour) instead of the portable lrintf path.
# Those live in asm/ftola.c + asm/snapvector.c, which the ARM/Ndk builds never compile
# (the whole file is #if defined(__i386__) || defined(__x86_64__)). Upstream ioquake3
# adds exactly these two for x86_64; the .asm JIT files stay out (NO_VM_COMPILED).
if(NOT EF_PLATFORM STREQUAL "android" AND
   CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x86_64-.*)$")
  list(APPEND SYS_SRC ${EF}/asm/ftola.c ${EF}/asm/snapvector.c)
endif()

# ---- renderers: modules selected at runtime by `cl_renderer` ----
#
# The renderer is not part of the client any more. It is a loadable module that
# CL_InitRef() dlopen()s once, exactly like ioquake3/Quake3e and the author's
# EF2 port: the engine keeps ioquake3's USE_RENDERER_DLOPEN machinery, and the
# renderer fills a refexport_t from GetRefAPI() given the engine's refimport_t.
#
#   Linux   : Release/renderer_vulkan.so          (beside voyagersp; Sys_LoadDll
#                                                    resolves it against Sys_BinaryPath)
#   Android : lib/<abi>/librenderer_vulkan.so     (AGP packages every SHARED target
#                                                    built from this project)
#
# A module cannot link the engine's qcommon, so it compiles the small shared
# set below plus renderercommon/tr_subs.c (Com_* shims over refimport_t) -- the
# same organisation as Quake3e/EF2 standalone renderers.
#
# EF_STATIC_RENDERER=ON folds the renderer back into the executable (a debug /
# failsafe build with no dlopen); cl_renderer is then ignored.
option(EF_STATIC_RENDERER "Link the Vulkan renderer into the client instead of building a module" OFF)

file(GLOB RENDERCOMMON_SRC ${EF}/renderercommon/*.c)
file(GLOB RENDERER_VK_SRC ${EF}/renderervk/*.c)
# tr_subs.c is the module shim (own target); shader_data.c (prebuilt SPIR-V) is
# #included by vk.c and must never be compiled separately.
list(REMOVE_ITEM RENDERER_VK_SRC
  ${EF}/renderervk/tr_subs.c
  ${EF}/renderervk/shaders/spirv/shader_data.c)

# Shared code every renderer module needs (not reachable through refimport_t).
# q_shared.c/q_math.c provide the Com_*/Q_* helpers, the math and the string
# routines the tr_* sources call directly; ftola/snapvector are the x86-64
# inline-asm Q_ftol/Q_SnapVector implementations q_shared.h selects with idx64.
set(RENDERER_LIB_SRC
  ${EF}/qcommon/q_shared.c
  ${EF}/qcommon/q_math.c)
if(NOT EF_PLATFORM STREQUAL "android" AND
   CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x86_64-.*)$")
  list(APPEND RENDERER_LIB_SRC ${EF}/asm/ftola.c ${EF}/asm/snapvector.c)
endif()

set(GLSL_SRC "")
set(EF_USE_VULKAN TRUE)

# ---- bundled libs ----
file(GLOB ZLIB_SRC ${EF}/zlib/*.c)
file(GLOB JPEG_SRC ${EF}/jpeg-8c/*.c)


# ---- single-player game/cgame bridge (compiled vs the SP game headers + compat shim) ----
# efgame tree sits four levels up from this src/ dir (…/android-port-Single-Player/efgame).
# Was hardcoded to $HOME/… which breaks on any machine where the repo lives elsewhere;
# derive it relatively so the build is location-independent.
set(EFG ${CMAKE_CURRENT_SOURCE_DIR}/../../../../efgame)
add_library(sp_game_bridge STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/sp_bridge.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/sp_ui_bridge.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/sp_clink.c)
target_include_directories(sp_game_bridge PRIVATE
  ${EFG}/android ${EFG}/src/game ${EFG}/src/cgame ${EFG}/src/ui
  ${EFG}/src/client ${EFG}/src/renderer ${EFG}/src/qcommon
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp)
target_compile_options(sp_game_bridge PRIVATE
  -include ${EFG}/android/ef_android_compat.h -fno-threadsafe-statics -fcommon -w)
target_compile_definitions(sp_game_bridge PRIVATE NDEBUG)

# ---- target: libmain.so (Android) or voyagersp (Linux) — identical sources ----
set(EF_TARGET_SRC
  ${CLIENT_SRC} ${QCOMMON_SRC} ${SERVER_SRC} ${BOTLIB_SRC}
  ${SDL_SRC} ${SYS_SRC} ${SYS_MAIN}
  ${GLSL_SRC}
  ${ZLIB_SRC} ${JPEG_SRC} ${LIBMAD_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/sp_integration.c
  # Desktop retail-data importer (Android is a no-op stub: ImportActivity
  # owns that flow on the APK side).  Kept in the shared source list so both
  # targets always compile the same tree.
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/data_import.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/bink_ff.c)

if(EF_STATIC_RENDERER)
  # Failsafe/debug layout: the renderer is part of the client (no module, no
  # dlopen). renderercommon is needed by renderervk's tr_* sources; tr_subs.c
  # is NOT (the engine's qcommon supplies those Com_* entry points).
  list(APPEND EF_TARGET_SRC ${RENDERER_VK_SRC} ${RENDERCOMMON_SRC})
endif()

if(EF_PLATFORM STREQUAL "android")
  add_library(main SHARED ${EF_TARGET_SRC})
  set(EF_TARGET main)
  # Rename sys_main.c's main() to SDL_main() so SDLActivity's bootstrap calls it.
  set_source_files_properties(${SYS_MAIN} PROPERTIES COMPILE_DEFINITIONS "main=SDL_main")
  set(EF_ARCH_STRING "${CMAKE_ANDROID_ARCH_ABI}")
else()
  add_executable(voyagersp ${EF_TARGET_SRC})
  set(EF_TARGET voyagersp)
  set(EF_ARCH_STRING "x86_64")
  # Desktop: the binary lands in <build>/Release/ beside the game modules, the
  # layout the launcher script expects (same shape as Provenance EF2's).
  set_target_properties(voyagersp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
endif()

target_include_directories(${EF_TARGET} PRIVATE
  ${EF} ${EF}/zlib ${EF}/jpeg-8c ${EF}/AL
  ${EF}/libmad-0.15.1b ${EF}/libmad-0.15.1b/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp)

# SDL + FFmpeg headers: vendored (Android) or the system copies (Linux).
if(EF_PLATFORM STREQUAL "android")
  target_include_directories(${EF_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/include)
else()
  target_include_directories(${EF_TARGET} PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

target_compile_definitions(${EF_TARGET} PRIVATE
  ELITEFORCE
  BOTLIB
  # MP3 voice-over: register the libmad-backed mp3 codec (snd_codec.c) and build
  # snd_codec_mp3.c against the vendored libmad (mad.h). FPM_64BIT = portable mode.
  USE_CODEC_MP3 USE_INTERNAL_MP3 FPM_64BIT
  $<$<NOT:$<BOOL:${EF_VM_JIT}>>:NO_VM_COMPILED>
  $<$<BOOL:${EF_USE_VULKAN}>:USE_VULKAN>
  $<$<BOOL:${EF_USE_VULKAN}>:USE_VULKAN_API>
  # The renderer is a dlopen()ed module unless the failsafe static build is
  # requested: the engine then loads it by cl_renderer and never links it.
  $<$<NOT:$<BOOL:${EF_STATIC_RENDERER}>>:USE_RENDERER_DLOPEN>
  USE_FLEXIBLE_DISPLAY=1
  USE_OPENAL USE_OPENAL_DLOPEN
  PRODUCT_VERSION="1.40"
  ARCH_STRING="${EF_ARCH_STRING}"
  USE_LOCAL_HEADERS=1)

if(EF_PLATFORM STREQUAL "android")
  # Game data lives in the app's OBB directory (standard Android home for big
  # game data; accessible by the app without any storage permission).
  target_compile_definitions(${EF_TARGET} PRIVATE
    ANDROID_DATA_ROOT="/sdcard/Android/obb/com.voyager.efsp")
endif()

target_compile_options(${EF_TARGET} PRIVATE
  # Implicit declarations are errors: a function implicitly declared as int
  # whose real return type is float or a pointer makes callers read the wrong
  # register (an undeclared Q_atof in renderervk corrupted every parsed shader
  # value this way).
  -fno-strict-aliasing -fvisibility=hidden -Werror=implicit-function-declaration
  # Always optimize the engine even in debug APKs: the bytecode INTERPRETER
  # (NO_VM_COMPILED runs cgame/qagame every frame), q_math and the renderer
  # backend are murderously slow at -O0, which is the panning-lag bottleneck.
  -O2 -fomit-frame-pointer)

if(EF_PLATFORM STREQUAL "android")
  find_library(GLES2_LIB GLESv2)
  find_library(GLES3_LIB GLESv3)
  find_library(LOG_LIB log)

  # FFmpeg (minimal: Bink video/audio only) for EF1 SP .bik movie playback.
  # --start-group avoids static-lib ordering issues; zlib symbols come from the
  # engine's own bundled zlib (do not also link system z -> duplicate symbols).
  # Per-ABI FFmpeg static libs: armv7 in ffmpeg/lib (build_ffmpeg_arm.sh), arm64 in
  # ffmpeg/lib64 (build_ffmpeg_arm64.sh). Headers (ffmpeg/include) are arch-independent.
  if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    set(FFLIB ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/lib64)
  else()
    set(FFLIB ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/lib)
  endif()
  set(FFMPEG_LIBS
    -Wl,--start-group
    ${FFLIB}/libavformat.a
    ${FFLIB}/libavcodec.a
    ${FFLIB}/libswscale.a
    ${FFLIB}/libavutil.a
    -Wl,--end-group)

  target_link_libraries(${EF_TARGET} sp_game_bridge SDL2 ${GLES3_LIB} ${GLES2_LIB} EGL ${LOG_LIB} ${log_lib} ${FFMPEG_LIBS} dl m)
else()
  # no link-time Vulkan dependency anywhere: the engine talks to the window via
  # SDL_Vulkan_* (SDL dlopens the loader itself) and the renderer resolves every
  # entry point through ri.VK_GetInstanceProcAddr. Only its headers are needed,
  # and those are vendored in renderercommon/vulkan/.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET
    libavformat libavcodec libswscale libavutil)

  target_link_libraries(${EF_TARGET} PRIVATE
    sp_game_bridge
    SDL2::SDL2
    PkgConfig::AVCODEC
    dl m pthread)

  # The SP game modules are dlopen'd by name ("libefgame.so") from the working
  # directory / LD_LIBRARY_PATH; keep the rpath to the build dir so a bare
  # ./voyagersp from Release/ finds them without an install step.
  set_target_properties(voyagersp PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN")
endif()

# ---------------------------------------------------------------------------
# Renderer modules
#
# Each backend is a loadable module the client selects once, at CL_InitRef(),
# from `cl_renderer` (default "vulkan"). The module exports GetRefAPI(); the
# engine hands it the refimport_t and gets the refexport_t back. A module is
# never linked against the client: it compiles its own qcommon maths and the
# tr_subs.c Com_* shims, and resolves every Vulkan entry point dynamically.
#
# Names (agreed contract, mirrors EF2's selection path):
#   Linux   : Release/renderer_vulkan.so
#   Android : lib/<abi>/librenderer_vulkan.so   ("lib<name>.so" is what the
#             Android packager accepts and what dlopen()s by bare name)
# cl_renderer holds the bare backend name ("vulkan"), never the file name.
# ---------------------------------------------------------------------------
if(NOT EF_STATIC_RENDERER)
  add_library(renderer_vulkan SHARED
    ${RENDERER_VK_SRC}
    ${RENDERCOMMON_SRC}
    ${RENDERER_LIB_SRC}
    ${EF}/qcommon/puff.c      # tr_image_png.c::R_LoadPNG uses the bundled inflate
    ${JPEG_SRC})              # tr_image_jpg.c links its own jpeg decoder copy

  # renderercommon/qgl.h still pulls SDL_opengl.h (the shared tr_image_*/tr_font
  # code includes qgl.h for the GL enum compatibility layer), so the module
  # needs the same SDL headers the client uses.
  if(EF_PLATFORM STREQUAL "android")
    target_include_directories(renderer_vulkan PRIVATE
      ${EF} ${EF}/zlib ${EF}/jpeg-8c
      ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
  else()
    target_include_directories(renderer_vulkan PRIVATE
      ${EF} ${EF}/zlib ${EF}/jpeg-8c ${SDL2_INCLUDE_DIRS})
  endif()

  target_compile_definitions(renderer_vulkan PRIVATE
    ELITEFORCE
    USE_VULKAN
    USE_RENDERER_DLOPEN
    USE_FLEXIBLE_DISPLAY=1
    USE_LOCAL_HEADERS=1
    PRODUCT_VERSION="1.40"
    ARCH_STRING="${EF_ARCH_STRING}")

  # Same hardening as the client: an implicit int declaration of a float- or
  # pointer-returning helper is a miscompiled read across the whole renderer
  # (the Q_atof white-fog bug was exactly that), so make it an error here too.
  target_compile_options(renderer_vulkan PRIVATE
    -fno-strict-aliasing -fvisibility=hidden
    -Werror=implicit-function-declaration
    -O2 -fomit-frame-pointer)

  if(EF_PLATFORM STREQUAL "android")
    # librenderer_vulkan.so -- AGP packages every SHARED target of this project
    # into the APK's lib/<abi>/ automatically.
    set_target_properties(renderer_vulkan PROPERTIES PREFIX "lib")
    target_link_libraries(renderer_vulkan PRIVATE m)
  else()
    # Release/renderer_vulkan.so beside voyagersp. Sys_LoadDll() resolves the
    # bare file name against Sys_BinaryPath(), so no rpath entry is needed.
    set_target_properties(renderer_vulkan PROPERTIES
      PREFIX ""
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
    target_link_libraries(renderer_vulkan PRIVATE dl m pthread)
  endif()
endif()

# ---------------------------------------------------------------------------
# renderer_gles3 -- the second renderer module (GLES 3.2)
#
# Status: GL layer + GLSL are ported (AUDIT.md in that directory); the drawing
# backend is not written yet, so the module reports renderer_backend_ready()=0.
# It is still a real module target on both platforms so the layer stays
# compiled, linked and loadable while the backend work lands. The client never
# offers it (cl_rendererModules lists ready modules only) and loading it
# manually makes the client fall back to Vulkan.
#
# Sources today: the EF2 GL entry-point loader, this engine's SDL/EGL-free GLES
# context layer and the module entry point. It carries its own q_shared.c so it
# has no link-time engine symbol, exactly like the Vulkan module.
# ---------------------------------------------------------------------------
set(RENDERER_GLES3_SRC
  ${EF}/renderergles3/qgl_gles3.c
  ${EF}/renderergles3/gl_gles3_platform.c
  ${EF}/renderergles3/gl3_renderer.c
  ${EF}/qcommon/q_shared.c)

if(NOT EF_STATIC_RENDERER)
  add_library(renderer_gles3 SHARED ${RENDERER_GLES3_SRC})

  if(EF_PLATFORM STREQUAL "android")
    target_include_directories(renderer_gles3 PRIVATE
      ${EF} ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include)
  else()
    target_include_directories(renderer_gles3 PRIVATE
      ${EF} ${SDL2_INCLUDE_DIRS})
  endif()

  # USE_VULKAN is required even though this backend has no Vulkan code: the
  # engine compiles refexport_t/refimport_t (renderercommon/tr_public.h) with
  # USE_VULKAN, which selects the Quake3e Cvar_CheckRange signature and the
  # VKimp_* window slots. A module built without it would have a different
  # struct layout than the client that fills it.
  target_compile_definitions(renderer_gles3 PRIVATE
    ELITEFORCE
    USE_VULKAN
    USE_RENDERER_DLOPEN
    USE_FLEXIBLE_DISPLAY=1
    USE_LOCAL_HEADERS=1
    PRODUCT_VERSION="1.40"
    ARCH_STRING="${EF_ARCH_STRING}")

  target_compile_options(renderer_gles3 PRIVATE
    -fno-strict-aliasing -fvisibility=hidden
    -Werror=implicit-function-declaration
    -O2 -fomit-frame-pointer)

  if(EF_PLATFORM STREQUAL "android")
    # librenderer_gles3.so, packaged by AGP from this target.
    set_target_properties(renderer_gles3 PROPERTIES PREFIX "lib")
    target_link_libraries(renderer_gles3 PRIVATE SDL2 m)
  else()
    # Release/renderer_gles3.so beside voyagersp.
    set_target_properties(renderer_gles3 PROPERTIES
      PREFIX ""
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
    target_link_libraries(renderer_gles3 PRIVATE SDL2::SDL2 dl m pthread)
  endif()
endif()
