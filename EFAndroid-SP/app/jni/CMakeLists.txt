cmake_minimum_required(VERSION 3.6)

project(GAME)

# Align shared-lib segments to 16 KB so the app is compatible with Android 15+
# 16 KB page-size devices (NDK 25 defaults to 4 KB). Applies to SDL2 + libmain.
add_link_options(-Wl,-z,max-page-size=16384)

# Strip the absolute host build path from __FILE__ / assert strings / debug info so
# the developer's home directory never leaks into libmain.so (or any log a user sends
# back). Applies to every target defined below (SDL + engine + bridge). -ffile-prefix-map
# covers debug info + __FILE__; -fmacro-prefix-map covers __FILE__ inside macros (assert).
add_compile_options(
  -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src
  -fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=/src)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()


# SDL sources are in a subfolder named "SDL"
add_subdirectory(SDL)

# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

# Your game and its CMakeLists.txt are in a subfolder named "src"
add_subdirectory(src)

