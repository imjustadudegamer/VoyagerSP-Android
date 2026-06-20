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

# ---- renderer: VULKAN-ONLY (Quake3e renderervk port — kills the Mali pipeline-revalidation panning
#      stutter). The dead OpenGL trees (renderergl1/renderergl2) were stripped: this SP port and the HM
#      port (renderervk + renderercommon only) both ship Vulkan exclusively.
#      The old EF_RENDERER=gl2|vulkan switch + the gl2 fallback branch are gone — there is no GL renderer
#      to fall back to. (renderercommon stays: renderervk's shared tr_*.c use its GL-enum/qgl compat.) ----
file(GLOB RENDERCOMMON_SRC ${EF}/renderercommon/*.c)
file(GLOB RENDERER_SRC ${EF}/renderervk/*.c)
# tr_subs.c (standalone-DLL shims) excluded — statically linked
list(REMOVE_ITEM RENDERER_SRC ${EF}/renderervk/tr_subs.c)
# shader_data.c (prebuilt SPIR-V) is #included by vk.c — must NOT compile separately
list(REMOVE_ITEM RENDERER_SRC ${EF}/renderervk/shaders/spirv/shader_data.c)
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

add_library(main SHARED
  ${CLIENT_SRC} ${QCOMMON_SRC} ${SERVER_SRC} ${BOTLIB_SRC}
  ${SDL_SRC} ${SYS_SRC} ${SYS_MAIN}
  ${RENDERER_SRC} ${RENDERCOMMON_SRC} ${GLSL_SRC}
  ${ZLIB_SRC} ${JPEG_SRC} ${LIBMAD_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/sp_integration.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp/bink_ff.c)

# Rename sys_main.c's main() to SDL_main() so SDLActivity's bootstrap calls it.
set_source_files_properties(${SYS_MAIN} PROPERTIES COMPILE_DEFINITIONS "main=SDL_main")

target_include_directories(main PRIVATE
  ${EF} ${EF}/zlib ${EF}/jpeg-8c ${EF}/AL
  ${EF}/libmad-0.15.1b ${EF}/libmad-0.15.1b/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../SDL/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../sp
  ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/include)

target_compile_definitions(main PRIVATE
  ELITEFORCE
  BOTLIB
  # MP3 voice-over: register the libmad-backed mp3 codec (snd_codec.c) and build
  # snd_codec_mp3.c against the vendored libmad (mad.h). FPM_64BIT = portable mode.
  USE_CODEC_MP3 USE_INTERNAL_MP3 FPM_64BIT
  $<$<NOT:$<BOOL:${EF_VM_JIT}>>:NO_VM_COMPILED>
  $<$<BOOL:${EF_USE_VULKAN}>:USE_VULKAN>
  $<$<BOOL:${EF_USE_VULKAN}>:USE_VULKAN_API>
  USE_FLEXIBLE_DISPLAY=1
  USE_OPENAL USE_OPENAL_DLOPEN
  PRODUCT_VERSION="1.40"
  ARCH_STRING="${CMAKE_ANDROID_ARCH_ABI}"
  # Game data lives in the app's OBB directory (standard Android home for big
  # game data; accessible by the app without any storage permission).
  ANDROID_DATA_ROOT="/sdcard/Android/obb/com.voyager.efsp"
  USE_LOCAL_HEADERS=1)

target_compile_options(main PRIVATE
  # Implicit declarations are errors: a function implicitly declared as int
  # whose real return type is float or a pointer makes callers read the wrong
  # register (an undeclared Q_atof in renderervk corrupted every parsed shader
  # value this way).
  -fno-strict-aliasing -fvisibility=hidden -Werror=implicit-function-declaration
  # Always optimize the engine even in debug APKs: the bytecode INTERPRETER
  # (NO_VM_COMPILED runs cgame/qagame every frame), q_math and the renderer
  # backend are murderously slow at -O0, which is the panning-lag bottleneck.
  -O2 -fomit-frame-pointer)

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

target_link_libraries(main sp_game_bridge SDL2 ${GLES3_LIB} ${GLES2_LIB} EGL ${LOG_LIB} ${log_lib} ${FFMPEG_LIBS} dl m)
