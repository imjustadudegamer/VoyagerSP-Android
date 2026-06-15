cmake_minimum_required(VERSION 3.16)
project(efsp_engine C CXX)

set(EFG <repo>/efgame)

# Headless SP engine (dedicated-style) reusing the HM-tweaked qcommon (FS/CM/cvar/cmd/common).
# First milestone: Com_Init -> FS mounts the real SP paks on device.
#
# Build (armv7):
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
#     -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-24
#   cmake --build build   -> build/efsp_headless

set(SH ${CMAKE_SOURCE_DIR}/engine-shared)

set(QCOMMON_SRC
  ${SH}/qcommon/common.c ${SH}/qcommon/cmd.c ${SH}/qcommon/cvar.c
  ${SH}/qcommon/files.c ${SH}/qcommon/unzip.c ${SH}/qcommon/ioapi.c
  ${SH}/qcommon/md4.c ${SH}/qcommon/md5.c ${SH}/qcommon/q_math.c ${SH}/qcommon/q_shared.c
  ${SH}/qcommon/cm_load.c ${SH}/qcommon/cm_patch.c ${SH}/qcommon/cm_polylib.c
  ${SH}/qcommon/cm_test.c ${SH}/qcommon/cm_trace.c
  ${SH}/qcommon/huffman.c ${SH}/qcommon/puff.c ${SH}/qcommon/msg.c
  ${SH}/qcommon/vm.c ${SH}/qcommon/vm_interpreted.c
)
set(SYS_SRC
  ${SH}/sys/sys_unix.c ${SH}/sys/con_passive.c ${SH}/sys/con_log.c
)
file(GLOB ZLIB_SRC ${SH}/zlib/*.c)

add_executable(efsp_headless
  ${CMAKE_SOURCE_DIR}/sp_headless_main.c
  ${CMAKE_SOURCE_DIR}/sp_stubs.c ${CMAKE_SOURCE_DIR}/sp_clink.c
  ${QCOMMON_SRC} ${SYS_SRC} ${ZLIB_SRC})

target_include_directories(efsp_headless PRIVATE
  ${SH}/qcommon ${SH}/sys ${SH}/zlib <hm-repo>/EFAndroid/app/jni/SDL/include ${SH}/client ${SH}/server ${SH}/botlib ${SH}/renderercommon)

target_compile_definitions(efsp_headless PRIVATE
  ELITEFORCE DEDICATED NO_VM_COMPILED BOTLIB
  ARCH_STRING="armeabi-v7a"
  ANDROID_DATA_ROOT="/data/local/tmp/efsp"
  USE_LOCAL_HEADERS=1)

target_compile_options(efsp_headless PRIVATE
  -fno-strict-aliasing -Wno-everything
  -Wl,-error-limit=0)

# The GetGameAPI bridge: compiled against the SP game headers (+compat shim), C++.
add_library(sp_bridge STATIC ${CMAKE_SOURCE_DIR}/sp_bridge.cpp)
target_include_directories(sp_bridge PRIVATE ${EFG}/android ${EFG}/src/game ${EFG}/src/cgame ${EFG}/src/ui ${EFG}/src/client ${EFG}/src/renderer ${EFG}/src/qcommon)
target_compile_options(sp_bridge PRIVATE -include ${EFG}/android/ef_android_compat.h -fno-threadsafe-statics -w)

target_link_options(efsp_headless PRIVATE -Wl,--error-limit=0)
target_link_libraries(efsp_headless sp_bridge m dl)
