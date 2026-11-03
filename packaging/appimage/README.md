# VoyagerSP — portable AppImage (Linux x86-64)

The distribution layer for the native Linux build of *Star Trek: Voyager — Elite
Force* (single player). It takes the one tree (`EFAndroid-SP/app/jni` +
`efgame/`) and produces a single relocatable file:

```
VoyagerSP-<version>-x86_64.AppImage
```

Everything here is additive: `packaging/` is the only directory this layer owns.
The engine, the game modules, the Android app and their CMake files are built,
never edited.

**The retail game data is not included and is never downloaded.** The owner of
the retail game supplies `BaseEF/pak0.pk3 … pak3.pk3` plus `efgamex86.dll`; the
engine's own first-run importer (the code path shared with the Android build)
verifies them and copies them into the per-user data root (see
[Data root and first run](#data-root-and-first-run)).

---

## Quick start

```sh
cd packaging/appimage
./setup-buildbox.sh      # once: Ubuntu 22.04 build box (glibc 2.35), SDL2 + FFmpeg
./build-appimage.sh     # build + stage + static verification + package
```

Output:

```
packaging/appimage/dist/VoyagerSP-1.2.0-x86_64.AppImage
packaging/appimage/dist/VoyagerSP-1.2.0-x86_64.AppImage.sha256
packaging/appimage/dist/VoyagerSP-1.2.0-x86_64.build-info.txt
```

`build-appimage.sh --help` lists the options (`--builder host` to skip the box,
`--no-build` to repackage without compiling, `--clean`, `--jobs`, `--version`).

Requirements for the *build*: `bwrap` (bubblewrap), `curl`, `sha256sum`, `git`,
`python3`, ~4 GB free disk, network access for the first `setup-buildbox.sh`
(≈ 250 MB of Ubuntu base + packages). **No sudo, no Docker, no root.**

Requirements for *running*: x86-64, glibc ≥ 2.35, a Vulkan 1.0+ driver, and an X11
display or XWayland (see the [matrix](#distribution-support)).

---

## How it works

1. **Tools** (`tools.lock`). Downloads `appimagetool` 1.9.1 and the
   type-2 runtime `20251108` once into `build/tools/`, verifies both against
   pinned sha256 (and the runtime against the AppImage project's GPG key,
   `570C77ACEA40C0F1B758902CBF96CCA56490F695`), and refuses to continue on any
   mismatch. The runtime is supplied with `--runtime-file`, so
   appimagetool never downloads "the latest runtime" behind our back.
2. **Build** (`build/engine`): one `cmake -DEF_PLATFORM=linux` configure of
   `EFAndroid-SP/app/jni`, which also pulls in `efgame/` on Linux, produces
   `Release/voyagersp`, `Release/libefgame.so` and `Release/libefui.so`.
3. **Snapshot** (`build/snapshot`): the three outputs are copied and re-hashed.
   The tree is under active development by another agent, so a half-written
   `.so` must never reach the image: a mismatch aborts the run.
4. **Stage** (`build/appdir/VoyagerSP.AppDir`): binary and modules into
   `usr/bin` / `usr/lib`, the dependency closure resolved with `ldd` and copied
   (minus the host-only list below), stripped, `$ORIGIN` RUNPATH applied with
   `patchelf`, plus `AppRun`, the desktop entry, AppStream metadata and icons.
5. **Static verification** — nothing is executed (see [Testing](#testing)).
   `ldd -r`, `readelf -d`, `objdump -T` (GLIBC and GLIBCXX symbol floors),
   `desktop-file-validate`, `appstreamcli validate-tree`, `sh -n AppRun`.
6. **Package**: `appimagetool --runtime-file <pinned> --comp zstd`, with
   `-all-time $SOURCE_DATE_EPOCH`, then a payload round-trip check via
   `--appimage-extract` and a BuildID check that the embedded runtime is the
   pinned one.

The whole run is idempotent: re-running prints the same image byte for byte
(same inputs, same `SOURCE_DATE_EPOCH`).

### Why appimagetool directly, not linuxdeploy

`linuxdeploy` is convenient but wrong for this tree: it resolves dependencies by
walking the *host* binary (which here would be the glibc-2.39 host build), it
applies its own bundled-library exclude list over which this pipeline has no say,
and its plugins download extra AppImages at build time (FUSE and network
dependent, non-reproducible). This AppDir is small and fully understood — five
libraries, decided by an explicit policy table — so the pipeline drives
`appimagetool` itself and owns every file that ends up in the image. The
excludelist reasoning (glibc, driver stack, libstdc++, X11/Wayland/audio) is
still the AppImage project's; it is encoded in `is_host_lib()` in
`build-appimage.sh` with the reasons next to it.

---

## The glibc floor: why an Ubuntu 22.04 build box

This host is Linux Mint 22.3 / Ubuntu 24.04, glibc 2.39. Linking here would make
the binary require `GLIBC_2.38`: glibc 2.38 re-versioned `fmodf` as the *default*
`fmodf`, so any program linked on a 2.38+ system that uses `fmodf()` (the engine
does, all over `q_math`) records `GLIBC_2.38`. That would lock out Debian 12
(2.36) and Ubuntu 22.04 (2.35).

`setup-buildbox.sh` therefore creates an unprivileged build box:

* Ubuntu **22.04.5** `ubuntu-base` rootfs (pinned sha256), extracted under
  `build/box/22.04/rootfs`;
* entered with `bwrap`: a user namespace maps the invoking user to uid 0 inside,
  the rootfs is bind-mounted as `/`, the repository is bind-mounted at the same
  absolute path (so build directories stay valid on both sides);
* packages installed *inside that rootfs only* with `apt`, using the documented
  `APT::Sandbox::User "root"` workaround (only uid 0 is mapped, so apt's normal
  privilege drop fails). Nothing on the host is touched: no sudo, no host
  packages, no loop mounts, no Docker.

Two libraries are built from source inside the box because Ubuntu 22.04's
versions do not fit the tree:

| library | why | source of truth |
|---|---|---|
| SDL2 **2.30.9** | the tree already vendors 2.30.9 for Android and the Linux CMake wants ≥ 2.0.22 (jammy ships 2.0.20); building it here also means the AppImage carries a known SDL rather than whatever the host has | `EFAndroid-SP/app/jni/SDL` |
| FFmpeg **6.1.1** | `sp/bink_ff.c` uses the FFmpeg ≥ 5.1 API; jammy ships 4.4. Configured minimal: `--disable-everything --disable-programs --disable-doc --disable-network --disable-autodetect --enable-demuxer=bink --enable-decoder=bink,binkaudio_rdft,binkaudio_dct --enable-protocol=file` plus shared libs/swscale; the build asserts the component lists really are 3 codecs / 1 demuxer / 1 protocol | `tools.lock` (pinned tarball + sha256) |

`setup-buildbox.sh` is idempotent and resumable: it downloads only what is
missing, skips `apt` when the packages are already configured, and only builds
SDL/FFmpeg when their marker files are absent (`--force-sdl`, `--force-ffmpeg`,
`--force-rootfs` to redo one).

### Measured result

```
file                GLIBC        GLIBCXX      RUNPATH
voyagersp           GLIBC_2.34   none         $ORIGIN/../lib:$ORIGIN
libefgame.so        GLIBC_2.27   GLIBCXX_3.4.29  $ORIGIN
libefui.so          GLIBC_2.27   none         $ORIGIN
max GLIBC required  GLIBC_2.35   (libavutil.so.58: hypot@GLIBC_2.35 — glibc 2.35
                                  re-versioned hypot; everything else needs less)
max GLIBCXX         GLIBCXX_3.4.29
```

`GLIBCXX_3.4.29` is GCC 11's libstdc++ — every distro with glibc 2.35 ships at
least that (Ubuntu 22.04 ships GCC 12 / 3.4.30).

### Fallback: build on the host

`--builder host` skips the box and builds with the host toolchain. That works and
is what the development launcher (`scripts/play_voyagersp.sh`) does, but the
result is much worse as a deliverable. Measured on this machine (Ubuntu 24.04,
reusing the box-built engine but taking every library from the host):

| | box build | host build |
|---|---|---|
| max `GLIBC_*` required | **2.35** | **2.38** |
| max `GLIBCXX_*` required | 3.4.29 | 3.4.32 |
| size | 3.9 MB | 64 MB |
| why | pinned minimal FFmpeg | the distro's FFmpeg drags its whole codec/network closure (gnutls, zmq, srt, ssh, openmpt, bluray, …) |

The host build therefore does not start on Debian 12 or Ubuntu 22.04. The script
prints the floor it measured and warns loudly. Use the box for anything that
leaves this machine.

---

## Distribution support

Claimed floor: **glibc 2.35**, libstdc++ **GLIBCXX_3.4.29**, x86-64, Vulkan, X11
or XWayland.

| distribution | glibc | verdict |
|---|---|---|
| Ubuntu 22.04 LTS / Linux Mint 21.x | 2.35 | supported (reference baseline) |
| Ubuntu 23.04 / 23.10 / 24.04 LTS / 24.10+ | 2.37 – 2.40 | supported |
| Linux Mint 22.x | 2.39 | supported |
| Debian 12 "bookworm" | 2.36 | supported |
| Debian 13 "trixie" | 2.41 | supported |
| Fedora 36+ | 2.35+ | supported |
| RHEL / CentOS Stream / Rocky 9 | 2.34 | **not supported** (glibc 2.34) |
| Debian 11 "bullseye" | 2.31 | **not supported** |
| openSUSE Leap 15.x | 2.31 | **not supported** |
| Arch / Manjaro / SteamOS 3 / openSUSE Tumbleweed | rolling (2.38+) | supported |
| Alpine / postmarketOS and anything musl | — | **not supported** (glibc build) |
| 32-bit, aarch64 | — | **not built** (x86-64 only) |

The floor was measured with `objdump -T` on every ELF in the image, not assumed.
Lowering it further (glibc 2.31 / Debian 11) would need a Bullseye box and older
SDL/FFmpeg pins; nothing in the tree requires that, so it is not attempted.

---

## What is inside

```
AppRun                                  the entry point (see below)
com.voyager.efsp.desktop                root copy, required by appimagetool
com.voyager.efsp.png                    root icon, required by appimagetool
.DirIcon -> com.voyager.efsp.png
usr/bin/voyagersp                       the engine (also the importer)
usr/lib/renderer_vulkan.so              the renderer module, dlopen()ed at CL_InitRef
usr/lib/renderer_gles3.so               second renderer module (GLES 3; inert, see below)
usr/lib/libefgame.so                    game + cgame + icarus (+speedrun), dlopen()ed
usr/lib/libefui.so                      the SP UI module, dlopen()ed
usr/lib/libSDL2-2.0.so.0                vendored SDL 2.30.9
usr/lib/libavformat.so.60 }             pinned minimal FFmpeg 6.1.1
usr/lib/libavcodec.so.60  }
usr/lib/libswscale.so.7   }
usr/lib/libavutil.so.58   }
usr/lib/vulkan/libvulkan.so.1           loader fallback, not on LD_LIBRARY_PATH
usr/share/applications/com.voyager.efsp.desktop
usr/share/metainfo/com.voyager.efsp.appdata.xml
usr/share/icons/hicolor/256x256/apps/com.voyager.efsp.png
usr/share/doc/voyagersp/{LICENSE,CREDITS.md,COPYING-NOTES.txt}
```

20 files, ~4 MB. The icon is the tree's own Android launcher icon
(`EFAndroid-SP/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png`), copied at build
time — no artwork was generated or captured for this.

### What is bundled, and why

| library | bundled? | reason |
|---|---|---|
| `libSDL2-2.0.so.0` | yes | version-controllable, vendored, and the distro SDL varies; the AppImage should not depend on a 2022 SDL |
| `libavformat` / `libavcodec` / `libswscale` / `libavutil` | yes | the SONAMEs differ across distros (`libavcodec` 58/59/60/61) — linking the host's is not an option |
| `libefgame.so`, `libefui.so` | yes | the engine `dlopen()`s them by bare name |
| `renderer_vulkan.so`, `renderer_gles3.so` | yes | the renderer is a loadable module now; the engine `dlopen()`s it by name and is fatal without it |
| `libvulkan.so.1` | **fallback only** | the loader must match the host's driver stack (see below). Neither the client nor `renderer_vulkan.so` links it any more — the client still calls `SDL_Vulkan_LoadLibrary`, so *SDL* `dlopen`s it at run time |
| glibc (`libc`, `libm`, `libdl`, `libpthread`, `librt`, `libresolv`, `ld-linux`) | no | always the host's; bundling glibc cannot work |
| `libstdc++.so.6`, `libgcc_s.so.1` | no | bundling an older libstdc++ shadows the host's for everything that `dlopen()`s into the process, and Mesa's Vulkan drivers are C++. The measured floor (3.4.29) is below every glibc-2.35 distro's. |
| X11 (`libX11`, `libXcursor`, `libXrandr`, `libXi`, `libXfixes`, `libXss`, `libxcb`), Wayland client | no | SDL loads them at run time; they must match the running display server |
| ALSA (`libasound`), PulseAudio (`libpulse`) | no | same: desktop-owned, loaded dynamically by SDL |
| `libz`, `libbz2`, `liblzma`, … | no | never needed: the FFmpeg build is `--disable-autodetect` and the engine carries its own zlib |
| `libopenal.so.1` | no | the engine's default is `s_useOpenAL 0` (`snd_main.c`), i.e. the built-in mixer over SDL audio; OpenAL is an opt-in that `dlopen()`s and falls back cleanly if absent. Bundling it would drag libsndfile/FLAC/libogg into the image for an opt-in path. |

### Renderer modules: where they go, and why

Since the client stopped linking the renderer, `CL_InitRef` builds
`renderer_<cl_renderer>.so` (Linux name, no `lib` prefix) and calls
`Sys_LoadDll(name, qtrue)`. The measured search order on Linux
(`efcode/sys/sys_main.c`) is:

1. `dlopen(name)` by bare name — resolved through the caller's `DT_RUNPATH`
   (`$ORIGIN/../lib:$ORIGIN` on the packaged engine) and `LD_LIBRARY_PATH`
   (AppRun sets it to `$APPDIR/usr/lib`);
2. `Sys_BinaryPath()/name` — the directory of `argv[0]`, i.e. `usr/bin/`;
3. `fs_basepath/name` — the user's data root.

The **CWD is never searched**, so a module found only by luck of the working
directory is not a concern here: the modules are installed into `usr/lib`
together with `libefgame.so`/`libefui.so`, and step 1 finds them by two
independent mechanisms (RUNPATH and `LD_LIBRARY_PATH`). A failure to load is
fatal in the engine (`Com_Error: Failed to load renderer`), which is why
`build-appimage.sh` refuses to package a `Release/` without `renderer_*.so`.

The pipeline stages **every** `renderer_*.so` the build produces, so adding or
removing a renderer is an engine-side decision, not a packaging one. On this tree
that is `renderer_vulkan.so` (the shipping backend, `cl_renderer vulkan`) and
`renderer_gles3.so` — the latter is **inert**: its backend is still being
written and it reports `renderer_backend_ready() = 0`, so the client never offers
it. If the engine stops building one of them, the image simply stops carrying it.

**The GPU driver is the host's, always.** The image contains no `libvulkan_*.so`,
no ICD JSON, no GL driver. `AppRun` prefers the host's `libvulkan.so.1` (found
via `ldconfig -p`) because loader and driver ship from the same stack; the
bundled copy in `usr/lib/vulkan/` is only used when a host has a driver but no
loader package (`SDL_VULKAN_LIBRARY` is pointed at it). Forcing the bundled
loader: `VOYAGERSP_BUNDLED_VULKAN=1`.

---

## Data root and first run

The first-run import is **engine code** (the same path the Android build uses),
not a shell script — the AppImage never implements discovery, validation or
copying of retail files.

`AppRun` only guarantees the environment that code needs:

```sh
VOYAGERSP_DATA_ROOT=${VOYAGERSP_DATA_ROOT:-${XDG_DATA_HOME:-$HOME/.local/share}/voyagersp}
mkdir -p "$VOYAGERSP_DATA_ROOT"
export VOYAGERSP_DATA_ROOT
exec "$APPDIR/usr/bin/voyagersp" "$@"
```

* The engine derives `fs_basepath` and `fs_homepath` from `VOYAGERSP_DATA_ROOT`
  itself; `AppRun` passes no `+set fs_basepath`, no `baseEF` symlink, and never
  points the engine at the retail installation.
* On first run the engine asks for `pak0.pk3 … pak3.pk3` and a genuine
  `efgamex86.dll` (`zenity`, then `kdialog`, then a terminal prompt), verifies
  them, symlinks a complete retail folder or copies the picked files into
  `<root>/baseEF/`, and continues into the game. Second run goes straight in.
* Arguments are forwarded verbatim — `--data-dir <path>`, `--import`,
  `--no-import`, `--import-from <dir>` and engine `+set` commands are the
  engine's.
* Import exit codes (the image propagates them): `0` ok, `2` cancelled, `3`
  invalid selection, `4` no picker available.

Nothing is ever written into the AppImage (read-only squashfs) or into the retail
installation.

The importer lives in `EFAndroid-SP/app/jni/sp/data_import.c` and is compiled
into `usr/bin/voyagersp`. The shipped image was checked (statically, after
`--appimage-extract`) to carry it: the binary contains `VOYAGERSP_DATA_ROOT`,
`--data-dir`, `--import`, `--no-import`, `--import-from`, `--import-only`,
`zenity`, `kdialog`, `baseEF` and `efgamex86.dll`, and the unstripped build
output exports the internal `DataImport_PreInit` symbol.

It needs **nothing extra in the bundle**: pickers are `fork`/`exec` of the host's
`zenity`/`kdialog` (optional; the engine falls back to a terminal prompt, then to
a plain error), and the file work is plain POSIX. If a host has neither picker
and no terminal, the import cannot be driven interactively — that is the
engine's documented exit code 4, not a packaging gap.

---

## Reproducibility

* `SOURCE_DATE_EPOCH` (default: the git commit time) is passed to mksquashfs as
  `-all-time`, so file timestamps in the payload do not vary.
* The runtime is pinned and verified, never fetched as "latest".
* Library closure and file set are deterministic (sorted `find`, no build-host
  paths in the binaries: the tree already maps `__FILE__` to `/src`, and the
  packaging build passes `CMAKE_SKIP_BUILD_RPATH=ON`).
* Verified on the current payload: independent packs of the same snapshot
  produce the identical sha256 recorded in `dist/*.sha256`, and so does a full
  rebuild from the same sources (the compile is deterministic too: rebuilding
  after unrelated edits gave byte-identical binaries).

Rebuilding the *engine* changes the payload by design (new hashes). The
`build-info.txt` records the input hashes so two images can be told apart.

---

## Testing

**This pipeline never runs the game and never runs the produced AppImage.** It
has no display, GPU or save-state isolation, and a botched run would take over
the desktop; verification is therefore static, and runtime testing is the user's
job.

Static checks the pipeline performs every run:

* `ldd` closure completeness (every `NEEDED` resolves) and `ldd -r` — 0
  undefined symbols on all three binaries;
* `readelf -d` — RUNPATH is `$ORIGIN/../lib:$ORIGIN` (engine) and `$ORIGIN`
  (modules); no `/home/...` or `/opt/...` paths;
* `objdump -T` — maximum `GLIBC_*` and `GLIBCXX_*` symbol versions per file and
  over the whole image;
* `desktop-file-validate` and `appstreamcli validate-tree`;
* `sh -n AppRun`;
* `appimagetool` payload round-trip: `--appimage-extract` and `diff -r` against
  the staged AppDir, plus the embedded runtime's ELF BuildID against the pin.

To test on a desktop:

```sh
chmod +x VoyagerSP-1.2.0-x86_64.AppImage
./VoyagerSP-1.2.0-x86_64.AppImage
```

If FUSE is missing (some newer distros do not install `libfuse2t64`
by default), either install it or run:

```sh
./VoyagerSP-1.2.0-x86_64.AppImage --appimage-extract-and-run
```

Useful while testing:

```sh
VOYAGERSP_DEBUG=1 ./VoyagerSP-1.2.0-x86_64.AppImage   # prints data root, then hands over
./VoyagerSP-1.2.0-x86_64.AppImage --apprun-help       # AppRun's own help
./VoyagerSP-1.2.0-x86_64.AppImage --apprun-version
./VoyagerSP-1.2.0-x86_64.AppImage --appimage-extract  # unpack to squashfs-root/
```

---

## Known limitations

* **No runtime test has been performed by the packaging pipeline.** The image is
  built and statically verified only; first launch on a real desktop is untested
  here.
* **X11 / XWayland only.** The vendored SDL is built with the X11 backend; pure
  Wayland sessions work through XWayland (present by default on GNOME/KDE).
  Enabling SDL's Wayland backend needs EGL headers at build time
  (`libegl1-mesa-dev`) and is not done.
* **Vulkan is required.** There is no OpenGL fallback in this port. Without a
  Vulkan driver the engine prints a renderer error and exits. Software Vulkan
  (`lavapipe`) works but is slow.
* **GPU driver from the host.** No ICD/driver is bundled.
* **OpenAL is optional and not bundled** (`+set s_useOpenAL 1` needs the host's
  `libopenal.so.1`); the default mixer over SDL audio needs nothing extra.
* **No update channel / signature.** No `-u` update information string is
  embedded (no hosting URL exists yet) and the image is not GPG-signed. Both are
  one flag away in `build-appimage.sh` when a release host exists.
* **No AppStream screenshots.** AppStream metadata intentionally carries none;
  producing one would mean running the game and capturing the screen, which this
  environment forbids. `appstreamcli validate-tree` passes without them.
* **x86-64 only.** No aarch64 cross-build; SDL/FFmpeg would need cross toolchains
  in the box.
* The version comes from `EFAndroid-SP/app/build.gradle` (`versionName`);
  `--version` overrides it.

---

## Tooling pins (`tools.lock`)

| tool | pin | integrity |
|---|---|---|
| appimagetool | 1.9.1 (`releases/download/1.9.1/appimagetool-x86_64.AppImage`) | sha256 `ed4ce84f…84eb0`; the asset carries no embedded GPG signature (`--appimage-signature` is empty), so the sha256 is the anchor |
| type-2 runtime | `20251108` (`runtime-x86_64`) | sha256 `2fca8b44…c260d` + GPG `good signature`, key `570C77AC…6490F695` (`AppImage type 2 runtime <type2-runtime@appimage.org>`) |
| Ubuntu base | 22.04.5 (`ubuntu-base-22.04-base-amd64.tar.gz`) | sha256 `df6fe77c…c4b07`, checked against Canonical's `SHA256SUMS` |
| FFmpeg | 6.1.1 (`ffmpeg-6.1.1.tar.xz`) | sha256 `8684f4b0…16f968` (ffmpeg.org also publishes a detached `.asc`) |
| SDL2 | 2.30.9 | from the tree, no download |

Everything downloaded lands in `build/` (gitignored) and is re-verified on every
run.

---

## Troubleshooting

| symptom | cause / fix |
|---|---|
| `AppImages require FUSE to run` | install `libfuse2`/`libfuse2t64`, or use `--appimage-extract-and-run` |
| `error while loading shared libraries: libX11.so.6` | no X11 client libraries; install `libx11-6` (or run under XWayland) |
| `error while loading shared libraries: libc.so.6: version GLIBC_2.35 not found` | host older than the floor; see the matrix |
| renderer initialises then `VK_ERROR_*` | no working Vulkan driver for the GPU; check `vulkaninfo` |
| `no system libvulkan.so.1 — using the bundled loader` | host has a driver but no loader package; install `libvulkan1` if the game then fails to enumerate devices |
| `Failed to load renderer "renderer_vulkan.so"` | the renderer module is missing from `usr/lib` (should never happen: the pipeline refuses to package without it, and the payload is verified after packing) |
| no sound | SDL found neither ALSA nor PulseAudio; or the album was muted with `+set s_initsound 0` |

Diagnostics that do not launch the game:

```sh
./VoyagerSP-*.AppImage --apprun-version                   # AppRun answers, then exits
./VoyagerSP-*.AppImage --appimage-extract                 # unpack to ./squashfs-root
ldd squashfs-root/usr/bin/voyagersp
readelf -d squashfs-root/usr/bin/voyagersp | grep RUNPATH
objdump -T squashfs-root/usr/bin/voyagersp | grep -o 'GLIBC_[0-9.]*' | sort -V | tail -1
appstreamcli validate-tree squashfs-root
```

---

## Files in this directory

| file | role |
|---|---|
| `build-appimage.sh` | the pipeline (build → stage → verify → package) |
| `setup-buildbox.sh` | creates/populates the unprivileged Ubuntu 22.04 box |
| `box-exec.sh` | runs one command inside the box (`bwrap` userns), used by both |
| `tools.lock` | every downloaded tool and checksum, sourced by the scripts |
| `appdir/AppRun` | the AppImage entry point (template; `__VOYAGERSP_VERSION__`) |
| `appdir/com.voyager.efsp.desktop` | desktop entry |
| `appdir/com.voyager.efsp.appdata.xml` | AppStream metadata (`.appdata.xml` because appimagetool 1.9.1 looks for that exact name) |
| `.gitignore` | keeps `build/`, `dist/`, `logs/` out of git |
