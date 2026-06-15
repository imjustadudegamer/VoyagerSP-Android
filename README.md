# Voyager Elite Force: Single Player for Android

A native Android port of *Star Trek: Voyager — Elite Force* — the **single-player
campaign** (2000). It is the single-player companion to the Holomatch (multiplayer)
port, and shares its engine base. Inspired by and building on
[VoyagerNX](https://github.com/faithvoid/VoyagerNX), faithvoid's Nintendo Switch port.
The engine base is [lilium-voyager](https://github.com/clover-moe/lilium-voyager) with
the Vulkan renderer from [Quake3e](https://github.com/ec-/Quake3e); the single-player
game logic (Raven's officially-released Elite Force SP source) runs as a native module.
Touch controls, full gamepad support.

No game data is included. You need your own copy of the retail game
(GOG release, original CD + patches, or an existing PC install).

## Features

- Vulkan renderer (Quake3e renderervk, adapted to the Elite Force engine) — this is the
  only renderer; the old OpenGL ES path has been removed.
- Native single-player game module (`libefgame` / `libefui`) built from Raven's Elite
  Force SP source for both `armeabi-v7a` and `arm64-v8a`. A width-explicit savegame
  transcoder keeps saves compatible across the 32-bit and 64-bit builds.
- Touch controls: virtual stick, look area, and an LCARS-styled button overlay that
  auto-hides while a gamepad is in use and during scripted cinematics.
- Gamepad: SDL GameController mapping, dual-analog defaults, with look-sensitivity and
  deadzone options.
- Aspect handling: 4:3 pillarbox or 16:9, applied to the 3D view, HUD, and 2D menus,
  with black side bars.
- Save menu thumbnails (real framebuffer capture) and autosave labelling.
- In-app game-data import: if the game data is missing at launch, the app lets you pick
  your `pak*.pk3` files and a retail `efgamex86.dll` with the system file picker and
  copies them into place. No storage permission needed at any target SDK.

## Screenshot from the first mission

<img width="1920" height="1080" alt="ScreenRecordofVoyagerSP - Trim - frame at 1m5s" src="https://github.com/user-attachments/assets/ca33430e-20ca-42ed-8d29-32836066d375" />

## Requirements

- Android 7.0+ (API 24) with Vulkan support — the APK declares Vulkan as a required
  feature.
- An ARM device. The APK is universal (`armeabi-v7a` + `arm64-v8a`).
- Retail Elite Force data: `pak0.pk3` (~541 MB) plus the patch paks `pak1.pk3`–`pak3.pk3`
  from `BaseEF/` of a PC installation, **and** a genuine `efgamex86.dll` from the game's
  install folder. The DLL is not loaded or executed by this port — it is required only as
  confirmation that you own a real copy of the game.

## Install

1. Copy your four `pak*.pk3` files and `efgamex86.dll` onto the phone (anywhere —
   Downloads is fine).
2. Install the APK and launch it.
3. When asked, pick the files. They are copied into the game's data directory and the
   game starts. That's the whole setup; next launches go straight into the game.

The manual push target (e.g. over adb) is
`/sdcard/Android/obb/com.voyager.efsp/baseEF/`.

## Known issues

This is an early release and has not been fully play-tested end to end. Known
limitations as of this build:

- **Loading screens are missing** — level transitions show no loading art yet.
- **Save corruption is possible** — back up your saves; loads may occasionally fail
  or restore an inconsistent state.
- **Minor graphical and audio glitches** are still present.
- **Character animations are not 100% accurate** yet.
- The game is **not fully play-tested**, so other issues may surface.

## Build

Everything needed to build is in this repository (engine, SDL2, the single-player game
source, and the prebuilt SPIR-V). No game data is required to build — only to run.

Prerequisites:

- Android SDK
- NDK `25.1.8937393`
- CMake `3.31.5`
- JDK 21

Point Gradle at your SDK with a `local.properties` file:

```sh
echo "sdk.dir=/path/to/Android/sdk" > EFAndroid-SP/local.properties
```

Build the native game/UI module, copy it into the app, then build the APK:

```sh
cmake --build efgame/build64 --target efgame efui      # arm64  (efgame/build for armv7)
cp efgame/build64/lib*.so EFAndroid-SP/app/libs/arm64-v8a/

cd EFAndroid-SP
./gradlew :app:assembleDebug    # output: app/build/outputs/apk/debug/app-debug.apk
```

The APK is universal (`armeabi-v7a` + `arm64-v8a`). Engine sources compiled into the APK
live in `EFAndroid-SP/app/jni/efcode/` (lilium-voyager plus the Android/Vulkan port
layer); the single-player game/UI module lives in `efgame/`.

## License & credits

This project is multi-licensed. The engine and port code are **GPLv2**
([LICENSE](LICENSE)) — from id Software's Quake III via ioquake3, lilium-voyager, and
Quake3e. The single-player game module (`efgame/`) is based on Raven Software's officially
released Elite Force SP source and is under Raven's **STEF Game Source License**
(`efgame/STEF Game Source License.doc`). *Star Trek* and related marks belong to
Paramount/CBS; this is a non-commercial fan port and includes no game assets. Full
attribution in [CREDITS.md](CREDITS.md).
