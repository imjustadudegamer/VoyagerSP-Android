# VoyagerSP-Android v1.1.6

Maintenance release for the _Star Trek: Voyager — Elite Force_ single-player Android port.

**Fixes**
* Fixed a crash on Mali and PowerVR phones when loading or changing levels — heavy map transitions could exhaust the GPU and close the game ("device lost").
* Reworked texture uploads to batch through the async upload queue instead of a full GPU wait per image, so map loads no longer flood weak mobile GPUs (the root cause of the crash above). No change on Adreno/other GPUs.
* Added automatic GPU-loss recovery as a safety net: if the graphics device is ever lost, the game now rebuilds the renderer and reloads your last autosave instead of crashing to the desktop.

Existing saves are unaffected.

**Requires your own retail game data** (`pak0.pk3`–`pak3.pk3` + a genuine `efgamex86.dll`; see README). No game assets bundled.
Universal APK (armeabi-v7a + arm64-v8a), release-signed. Vulkan required, Android 7.0+ (API 24).
