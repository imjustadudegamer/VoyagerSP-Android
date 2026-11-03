#!/usr/bin/env bash
# build-appimage.sh — build VoyagerSP (engine + game modules) and pack it as a
# portable type-2 AppImage.
#
#   ./setup-buildbox.sh      # once: Ubuntu 22.04 build box (glibc 2.35 floor)
#   ./build-appimage.sh      # build + stage + verify + package
#
# The pipeline, in order:
#   1. verify/download the pinned appimagetool + type-2 runtime (tools.lock);
#   2. configure and build EFAndroid-SP/app/jni with -DEF_PLATFORM=linux into
#      build/engine — one configure produces Release/voyagersp, Release/libefgame.so
#      and Release/libefui.so (the jni CMakeLists add_subdirectory()s efgame on Linux);
#   3. snapshot those three files (and their hashes) so packaging never races a
#      concurrent build of the same tree;
#   4. stage an AppDir: binary, modules, the non-glibc shared libraries they need,
#      stripped, with $ORIGIN rpaths, AppRun, .desktop, AppStream metadata, icons;
#   5. static verification: ldd, readelf -d, maximum GLIBC_* and GLIBCXX_* symbol
#      versions, no absolute build paths;
#   6. appimagetool with a pinned runtime, zstd squashfs and a fixed mksquashfs
#      timestamp, so two builds of the same inputs are byte-identical;
#   7. static verification of the produced image (payload round-trip via
#      --appimage-extract — never executed).
#
# This script NEVER runs the game and never runs the produced AppImage: the binary
# needs a display, a GPU and a Vulkan driver, and launching it is the user's job
# (see README.md, "Testing").
#
# Options:
#   --builder auto|box|host  where to compile (default auto: box when it exists)
#   --no-build               package the existing build/engine Release/ outputs
#   --clean                  remove the build dir, staged AppDir and dist outputs
#   --jobs N                 parallel compile jobs (default: nproc)
#   --version V              override the version (default: Android versionName)
#   -h, --help

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
# shellcheck source=tools.lock
. "$HERE/tools.lock"

BUILD=$HERE/build
ENGINE_BUILD=$BUILD/engine
RELEASE=$ENGINE_BUILD/Release
SNAP=$BUILD/snapshot
APPDIR=$BUILD/appdir/VoyagerSP.AppDir
DIST=$HERE/dist
LOGS=$HERE/logs
TOOLS=$BUILD/tools
BOX_DIR=${VOYAGERSP_BOX_DIR:-$BUILD/box/22.04}
ROOTFS=$BOX_DIR/rootfs
BOX_SDL_PREFIX=/opt/sdl2
BOX_FFMPEG_PREFIX=/opt/ffmpeg

BUILDER=auto
DO_BUILD=1
DO_CLEAN=0
JOBS=$(nproc)
VERSION_OVERRIDE=

APP_ID=com.voyager.efsp
APP_NAME=VoyagerSP

log()  { printf '\n== %s\n' "$*"; }
info() { printf '   %s\n' "$*"; }
warn() { printf '!! %s\n' "$*" >&2; }
die()  { printf 'build-appimage: %s\n' "$*" >&2; exit 1; }

while [ $# -gt 0 ]; do
    case $1 in
        --builder=*)  BUILDER=${1#*=}; shift ;;
        --builder)    BUILDER=${2:-auto}; shift 2 ;;
        --no-build)   DO_BUILD=0; shift ;;
        --clean)      DO_CLEAN=1; shift ;;
        --jobs=*)     JOBS=${1#*=}; shift ;;
        --jobs)       JOBS=${2:-$(nproc)}; shift 2 ;;
        --version=*)  VERSION_OVERRIDE=${1#*=}; shift ;;
        --version)    VERSION_OVERRIDE=${2:-}; shift 2 ;;
        -h|--help)    sed -n '2,38p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) die "unknown argument '$1' (see --help)" ;;
    esac
done
case $BUILDER in auto|box|host) ;; *) die "--builder must be auto, box or host" ;; esac

sha256_of() { sha256sum "$1" | awk '{print $1}'; }

# Version comparison on the GLIBC_x.y values (sort -V, not lexicographic).
ver_gt() {   # ver_gt A B -> true when A > B
    [ "$1" != "$2" ] && [ "$(printf '%s\n%s\n' "$1" "$2" | sort -V | tail -1)" = "$1" ]
}

# Run a command in the chosen build environment. Everything that touches the
# compiler, ldd, strip or the box's libraries goes through this.
run() {
    if [ "$BUILDER" = box ]; then
        "$HERE/box-exec.sh" -- "$@"
    else
        "$@"
    fi
}

# The box sees the repo at the same path; an absolute path *outside* the repo
# maps to the rootfs on the host side.
to_host_path() {
    case $1 in
        "$BUILD"/*) printf '%s\n' "$1" ;;
        *) [ "$BUILDER" = box ] && printf '%s%s\n' "$ROOTFS" "$1" || printf '%s\n' "$1" ;;
    esac
}

# Resolve the libraries we build against (both built into the box, neither in the
# loader's default search path) so ldd sees them before they are bundled.
ldd_cmd() {
    if [ "$BUILDER" = box ]; then
        printf '%s\0' env "LD_LIBRARY_PATH=$BOX_SDL_PREFIX/lib:$BOX_FFMPEG_PREFIX/lib" ldd "$1"
    else
        printf '%s\0' ldd "$1"
    fi
}

# ------------------------------------------------------------------ version
detect_version() {
    [ -n "$VERSION_OVERRIDE" ] && { printf '%s\n' "$VERSION_OVERRIDE"; return; }
    [ -n "${VERSION:-}" ] && { printf '%s\n' "$VERSION"; return; }
    local g=$REPO/EFAndroid-SP/app/build.gradle v
    if [ -f "$g" ]; then
        v=$(sed -n 's/^[[:space:]]*versionName[[:space:]]*"\([^"]*\)".*/\1/p' "$g" | head -1)
        [ -n "$v" ] && { printf '%s\n' "$v"; return; }
    fi
    printf '0.0.0\n'
}

VERSION=$(detect_version)
APPIMAGE_NAME="$APP_NAME-$VERSION-x86_64.AppImage"
APPIMAGE=$DIST/$APPIMAGE_NAME

# Fixed timestamp for mksquashfs: the commit time, so re-packaging the same tree
# twice gives the same image. Overridable the usual way.
if [ -n "${SOURCE_DATE_EPOCH:-}" ]; then
    EPOCH=$SOURCE_DATE_EPOCH
elif git -C "$REPO" rev-parse --git-dir >/dev/null 2>&1; then
    EPOCH=$(git -C "$REPO" log -1 --format=%ct)
else
    EPOCH=0
fi
BUILD_DATE=$(date -u -d "@$EPOCH" +%Y-%m-%d 2>/dev/null || date -u +%Y-%m-%d)

mkdir -p "$LOGS" "$DIST"

if [ "$DO_CLEAN" -eq 1 ]; then
    log "cleaning"
    rm -rf "$ENGINE_BUILD" "$SNAP" "$APPDIR"
    rm -f "$DIST"/*
    info "removed build/engine, build/snapshot, build/appdir and dist/*"
fi

# -------------------------------------------------------------------- tools
verify_sha256() {
    local f=$1 want=$2 got
    [ -f "$f" ] || die "missing $f"
    got=$(sha256_of "$f")
    [ "$got" = "$want" ] || die "checksum mismatch on $f
  expected $want
  got      $got"
}

ensure_tools() {
    mkdir -p "$TOOLS"
    log "tools (pinned in tools.lock)"
    if [ ! -f "$TOOLS/appimagetool-x86_64.AppImage" ]; then
        info "downloading appimagetool $APPIMAGETOOL_VERSION"
        curl -fL --retry 3 -o "$TOOLS/appimagetool-x86_64.AppImage.part" "$APPIMAGETOOL_URL"
        mv "$TOOLS/appimagetool-x86_64.AppImage.part" "$TOOLS/appimagetool-x86_64.AppImage"
    fi
    verify_sha256 "$TOOLS/appimagetool-x86_64.AppImage" "$APPIMAGETOOL_SHA256"
    chmod +x "$TOOLS/appimagetool-x86_64.AppImage"
    info "appimagetool sha256 ok: $APPIMAGETOOL_SHA256"

    if [ ! -f "$TOOLS/runtime-x86_64" ]; then
        info "downloading type-2 runtime $RUNTIME_RELEASE"
        curl -fL --retry 3 -o "$TOOLS/runtime-x86_64.part" "$RUNTIME_URL"
        mv "$TOOLS/runtime-x86_64.part" "$TOOLS/runtime-x86_64"
    fi
    verify_sha256 "$TOOLS/runtime-x86_64" "$RUNTIME_SHA256"
    info "runtime sha256 ok: $RUNTIME_SHA256"

    # The runtime is GPG-signed. Verify it when gpg is available; a missing gpg is
    # not fatal (the sha256 above is the hard check), but a *bad* signature is.
    if command -v gpg >/dev/null 2>&1; then
        [ -s "$TOOLS/signing-pubkey.asc" ] \
            || curl -fsSL --retry 3 -o "$TOOLS/signing-pubkey.asc" "$RUNTIME_PUBKEY_URL" || true
        [ -s "$TOOLS/runtime-x86_64.sig" ] \
            || curl -fsSL --retry 3 -o "$TOOLS/runtime-x86_64.sig" "$RUNTIME_SIG_URL" || true
        if [ -s "$TOOLS/signing-pubkey.asc" ] && [ -s "$TOOLS/runtime-x86_64.sig" ]; then
            local gh
            gh=$(mktemp -d)
            GNUPGHOME=$gh gpg --batch --import "$TOOLS/signing-pubkey.asc" >/dev/null 2>&1 || true
            if GNUPGHOME=$gh gpg --batch --verify "$TOOLS/runtime-x86_64.sig" "$TOOLS/runtime-x86_64" >/dev/null 2>&1; then
                info "runtime signature ok ($RUNTIME_SIGNING_FPR)"
            else
                rm -rf "$gh"
                die "runtime GPG verification FAILED — refusing to package with it"
            fi
            rm -rf "$gh"
        else
            warn "runtime signature files unavailable; relying on the pinned sha256"
        fi
    fi
}

# ------------------------------------------------------------------- builder
select_builder() {
    if [ "$BUILDER" = auto ]; then
        if [ -x "$ROOTFS/bin/bash" ] && [ -e "$ROOTFS$BOX_SDL_PREFIX/.voyagersp-built" ]; then
            BUILDER=box
        else
            BUILDER=host
        fi
    fi
    if [ "$BUILDER" = box ]; then
        [ -x "$ROOTFS/bin/bash" ] || die "--builder box: no rootfs at $ROOTFS (run setup-buildbox.sh)"
        [ -e "$ROOTFS$BOX_SDL_PREFIX/.voyagersp-built" ] \
            || die "--builder box: vendored SDL2 is not built in the box (run setup-buildbox.sh)"
        "$HERE/box-exec.sh" --check >/dev/null || die "--builder box: box is not usable"
        info "builder: Ubuntu $UBUNTU_BASE_VERSION box ($ROOTFS), glibc $UBUNTU_BASE_GLIBC"
    else
        info "builder: host ($( . /etc/os-release; echo "$PRETTY_NAME" ))"
        warn "host build: the glibc symbol floor will be this host's, not $UBUNTU_BASE_GLIBC —"
        warn "run setup-buildbox.sh to build inside the 22.04 box for older distros."
    fi
}

# --------------------------------------------------------------------- build
build_all() {
    [ -f "$REPO/EFAndroid-SP/app/jni/CMakeLists.txt" ] || die "no EFAndroid-SP/app/jni/CMakeLists.txt"
    [ -f "$REPO/efgame/CMakeLists.txt" ] || die "no efgame/CMakeLists.txt"

    # A CMake cache must never survive a change of the library environment: the
    # pkg-config results (absolute .so paths and version numbers) are cached
    # independently of each other, so switching builder or FFmpeg prefix in place
    # silently links a mix of the two. Fingerprint the environment and start over
    # when it differs.
    local fingerprint="builder=$BUILDER sdl=$BOX_SDL_PREFIX ffmpeg=$BOX_FFMPEG_PREFIX"
    if [ -d "$ENGINE_BUILD" ] && [ ! -f "$ENGINE_BUILD/.fingerprint" ]; then
        info "build dir of unknown provenance — clean build dir"
        rm -rf "$ENGINE_BUILD"
    elif [ -f "$ENGINE_BUILD/.fingerprint" ] && [ "$(cat "$ENGINE_BUILD/.fingerprint")" != "$fingerprint" ]; then
        info "library environment changed since the last configure — clean build dir"
        rm -rf "$ENGINE_BUILD"
    fi
    mkdir -p "$ENGINE_BUILD"
    echo "$fingerprint" > "$ENGINE_BUILD/.fingerprint"

    local cmake_args=(
        -S "$REPO/EFAndroid-SP/app/jni"
        -B "$ENGINE_BUILD"
        -G Ninja
        -DEF_PLATFORM=linux
        -DCMAKE_BUILD_TYPE=Release
        # No build-time rpath: CMake must not add the build host's /opt/sdl2 and
        # /opt/ffmpeg paths to the binaries' RUNPATH, and the Ninja generator
        # expands "$ORIGIN" inside raw linker flags (it treats '$' as its own
        # escape), so the relocatable rpath is written with patchelf after
        # stripping instead.
        -DCMAKE_SKIP_BUILD_RPATH=ON
    )
    local cfg_prefix=()
    if [ "$BUILDER" = box ]; then
        # Both are built by setup-buildbox.sh and neither is in pkg-config's or the
        # loader's default search path. Without SDL2_DIR the box's libsdl2-dev
        # (2.0.20) would fail the tree's find_package(SDL2 2.0.22); without the
        # PKG_CONFIG_LIBDIR the box's FFmpeg 4.4 would win over the pinned 6.1.1.
        cmake_args+=(
            "-DSDL2_DIR=$BOX_SDL_PREFIX/lib/cmake/SDL2"
            "-DCMAKE_PREFIX_PATH=$BOX_SDL_PREFIX;$BOX_FFMPEG_PREFIX"
        )
        cfg_prefix=(env "PKG_CONFIG_LIBDIR=$BOX_FFMPEG_PREFIX/lib/pkgconfig")
    fi

    log "configure (EF_PLATFORM=linux)"
    run "${cfg_prefix[@]}" cmake "${cmake_args[@]}" > "$LOGS/configure.log" 2>&1 \
        || { tail -25 "$LOGS/configure.log" >&2; die "cmake configure failed (logs/configure.log)"; }
    grep -E 'EF_PLATFORM|SDL2|Vulkan|libav|efgame:' "$LOGS/configure.log" | sed 's/^-- /   /' || true

    # Guard against silently linking the box's distro FFmpeg 4.4 (libavcodec 58)
    # instead of the pinned 6.1.1 (libavcodec 60): the engine needs the >= 5.1 API.
    # Read the resolved values out of the cache, not the log: an incremental
    # configure prints nothing for already-cached checks.
    local av_lib av_ver
    av_lib=$(sed -n 's/^pkgcfg_lib_AVCODEC_avcodec:FILEPATH=//p' "$ENGINE_BUILD/CMakeCache.txt")
    av_ver=$(sed -n 's/^AVCODEC_libavcodec_VERSION:INTERNAL=//p' "$ENGINE_BUILD/CMakeCache.txt")
    info "ffmpeg: libavcodec $av_ver ($av_lib)"
    case $av_ver in
        6[0-9].*) : ;;
        *) die "the build did not pick up FFmpeg 6.x (got '$av_ver' from '$av_lib') — see logs/configure.log" ;;
    esac
    if [ "$BUILDER" = box ]; then
        case $av_lib in
            "$BOX_FFMPEG_PREFIX"/*) : ;;
            *) die "linked $av_lib instead of the pinned $BOX_FFMPEG_PREFIX build" ;;
        esac
    fi

    log "build (-j$JOBS)"
    run cmake --build "$ENGINE_BUILD" -j "$JOBS" > "$LOGS/build.log" 2>&1 \
        || { tail -30 "$LOGS/build.log" >&2; die "build failed (logs/build.log)"; }
    info "build log: $(grep -ci warning "$LOGS/build.log" || true) warning line(s), $(wc -l < "$LOGS/build.log") lines"

    local f
    for f in voyagersp libefgame.so libefui.so; do
        [ -f "$RELEASE/$f" ] || die "expected build output is missing: $RELEASE/$f"
    done
    info "outputs: $(cd "$RELEASE" && ls -l voyagersp libefgame.so libefui.so | awk '{printf "%s=%s ", $9, $5}')"
}

snapshot_outputs() {
    log "snapshot of the build outputs"
    rm -rf "$SNAP"
    mkdir -p "$SNAP"
    local f
    for f in voyagersp libefgame.so libefui.so; do
        # Copy, then confirm the copy matches what was read: the tree may be rebuilt
        # by another agent while we package, and a half-written .so must never ship.
        cp -p "$RELEASE/$f" "$SNAP/$f"
        [ "$(sha256_of "$RELEASE/$f")" = "$(sha256_of "$SNAP/$f")" ] \
            || die "$f changed while it was being copied; re-run the build"
        printf '   %-14s %10s bytes  sha256 %s\n' "$f" "$(stat -c%s "$SNAP/$f")" "$(sha256_of "$SNAP/$f")"
    done
}

# ---------------------------------------------------------------- AppDir stage
# Libraries that must come from the host, with the reason. Everything else in the
# dependency closure is bundled. Matching is on the SONAME (basename).
#
# Deliberately NOT bundled:
#   * glibc and its parts — the loader is the host's, always.
#   * the GL/Vulkan *driver* stack — the loader reads the host's
#     /usr/share/vulkan/icd.d and loads the host driver; bundling a driver or a
#     loader that shadows the host one lies about what the machine can do. The
#     loader alone gets a fallback copy in usr/lib/vulkan (never on
#     LD_LIBRARY_PATH, see AppRun).
#   * libstdc++/libgcc_s — bundling an older libstdc++ shadows the host's and
#     breaks C++ host components that dlopen into our process (Mesa's Vulkan
#     drivers are C++). The box build's floor is GLIBCXX_3.4.29, which every
#     distro with glibc 2.35 also has; the build verifies this.
#   * X11 / Wayland client / ALSA / PulseAudio — desktop-essential, configured by
#     the host, and their version must match the running display/audio server.
#   * zlib — ancient, every glibc system has it.
is_host_lib() {
    case $1 in
        # glibc
        libc.so.6|libm.so.6|libdl.so.2|libpthread.so.0|librt.so.1|libresolv.so.2|\
        libutil.so.1|libanl.so.1|libmvec.so.1|libthread_db.so.1|libBrokenLocale.so.1|\
        libcidn.so.1|libcrypt.so.1|ld-linux-x86-64.so.2|libnss_*.so.2) return 0 ;;
        # GPU / display driver stack
        libGL.so.1|libEGL.so.1|libGLX.so.0|libGLdispatch.so.0|libOpenGL.so.0|\
        libgbm.so.1|libdrm.so.2|libglapi.so.0|libvulkan.so.1|libvulkan.so|\
        libvdpau.so.1|libva.so.2|libva-drm.so.2) return 0 ;;
        # X11 / Wayland / audio servers
        libX11.so.6|libX11-xcb.so.1|libxcb.so.1|libxcb-*.so.0|libXext.so.6|libXau.so.6|\
        libXdmcp.so.6|libwayland-client.so.0|libwayland-egl.so.1|libwayland-cursor.so.0|\
        libasound.so.2|libpulse.so.0|libpulse-simple.so.0|libpipewire-0.3.so.0|libjack.so.0|\
        libudev.so.1|libdbus-1.so.3) return 0 ;;
        # C++/gcc runtime — see the note above
        libstdc++.so.6|libgcc_s.so.1) return 0 ;;
        # zlib
        libz.so.1|libz.so) return 0 ;;
    esac
    return 1
}

stage_libs() {
    # Breadth-first over the dependency closure: ldd each file, copy every
    # resolved library that is not on the host-only list, then ldd the copies.
    # ldd already reports transitive dependencies, but walking explicitly means
    # the log states where each library came from and nothing is copied twice.
    local queue=() copied=() skipped=()
    local f name path dest ldd_out cmd=()
    queue=("$APPDIR/usr/bin/voyagersp" "$APPDIR/usr/lib/libefgame.so" "$APPDIR/usr/lib/libefui.so")

    while [ ${#queue[@]} -gt 0 ]; do
        f=${queue[0]}
        queue=("${queue[@]:1}")
        mapfile -d '' cmd < <(ldd_cmd "$f")
        if ! ldd_out=$(run "${cmd[@]}" 2>/dev/null); then
            warn "ldd failed on $f"
            continue
        fi
        while read -r name path; do
            [ -n "$name" ] || continue
            if [ ${#copied[@]} -gt 0 ] && [[ " ${copied[*]} " == *" $name "* ]]; then continue; fi
            if is_host_lib "$name"; then
                skipped+=("$name")
                continue
            fi
            if [ ! -e "$(to_host_path "$path")" ]; then
                warn "$name resolved to $path which is not readable; not bundled"
                skipped+=("$name")
                continue
            fi
            dest=$APPDIR/usr/lib/$name
            cp -L "$(to_host_path "$path")" "$dest"
            chmod 0644 "$dest"
            copied+=("$name")
            info "bundled $name  <- $path"
            queue+=("$dest")
        done < <(printf '%s\n' "$ldd_out" | awk '/=> \// {print $1, $3} /^\/.*\.so/ {print $1, $1}')
    done

    # SDL is the one library whose *version* we deliberately control (2.30.9 from
    # the tree, not the host's), so make its provenance explicit.
    if [ -e "$APPDIR/usr/lib/libSDL2-2.0.so.0" ]; then
        info "SDL2 bundled: $SDL2_VERSION (vendored, built in the box)"
    else
        warn "libSDL2-2.0.so.0 was not found in the closure — the binary will need the host's SDL"
    fi
    printf '%s\n' "${skipped[@]-}" | sort -u > "$BUILD/host-libs.txt"
    info "host-provided: $(printf '%s\n' "${skipped[@]-}" | sort -u | grep -c . || true) library name(s), see build/host-libs.txt"
}

stage_appdir() {
    log "staging AppDir"
    rm -rf "$APPDIR"
    mkdir -p "$APPDIR"/usr/bin "$APPDIR"/usr/lib \
             "$APPDIR"/usr/share/applications "$APPDIR"/usr/share/metainfo \
             "$APPDIR"/usr/share/icons/hicolor/256x256/apps \
             "$APPDIR"/usr/share/doc/voyagersp "$APPDIR"/usr/lib/vulkan

    install -m 0755 "$SNAP/voyagersp"    "$APPDIR/usr/bin/voyagersp"
    install -m 0755 "$SNAP/libefgame.so" "$APPDIR/usr/lib/libefgame.so"
    install -m 0755 "$SNAP/libefui.so"   "$APPDIR/usr/lib/libefui.so"

    # --- icon: the Android launcher icon, the only artwork the tree ships ------
    local icon=$REPO/EFAndroid-SP/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png
    [ -f "$icon" ] || die "missing app icon: $icon"
    install -m 0644 "$icon" "$APPDIR/$APP_ID.png"
    install -m 0644 "$icon" "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_ID.png"
    ln -sf "$APP_ID.png" "$APPDIR/.DirIcon"

    # --- metadata -------------------------------------------------------------
    sed "s/__VOYAGERSP_VERSION__/$VERSION/g" "$HERE/appdir/AppRun" > "$APPDIR/AppRun"
    chmod 0755 "$APPDIR/AppRun"
    install -m 0644 "$HERE/appdir/$APP_ID.desktop" "$APPDIR/$APP_ID.desktop"
    install -m 0644 "$HERE/appdir/$APP_ID.desktop" "$APPDIR/usr/share/applications/$APP_ID.desktop"
    # AppStream metadata. appimagetool hardcodes the legacy file name
    # (<desktop-id>.appdata.xml) when it looks the component up, so that is the
    # name used here; AppStream itself accepts .appdata.xml and .metainfo.xml
    # interchangeably and appstreamcli validates the tree either way.
    sed -e "s/__VOYAGERSP_VERSION__/$VERSION/g" -e "s/__VOYAGERSP_DATE__/$BUILD_DATE/g" \
        "$HERE/appdir/$APP_ID.appdata.xml" > "$APPDIR/usr/share/metainfo/$APP_ID.appdata.xml"

    # --- licences -------------------------------------------------------------
    install -m 0644 "$REPO/LICENSE" "$APPDIR/usr/share/doc/voyagersp/LICENSE"
    [ -f "$REPO/CREDITS.md" ] && install -m 0644 "$REPO/CREDITS.md" "$APPDIR/usr/share/doc/voyagersp/CREDITS.md"
    cat > "$APPDIR/usr/share/doc/voyagersp/COPYING-NOTES.txt" <<'EOF'
VoyagerSP is distributed under the GNU General Public License, version 2 (see LICENSE).

The bundled shared libraries are third-party components under their own licences:
  SDL2         zlib licence (https://libsdl.org)
  FFmpeg       LGPL-2.1-or-later (https://ffmpeg.org)
  libdecor     MIT (https://gitlab.freedesktop.org/libdecor/libdecor)
  libxkbcommon MIT/X11 (https://xkbcommon.org)
The Vulkan loader (usr/lib/vulkan, emergency fallback only) is Apache-2.0.

The retail game data (BaseEF/pak*.pk3) is NOT part of this image and is not
redistributable: it must be supplied by the owner of the retail game.
EOF

    stage_libs

    # --- Vulkan loader fallback ----------------------------------------------
    # Kept out of usr/lib on purpose: it is only used when the host has none (see
    # AppRun). Bundling the loader is a last resort, never the default.
    local loader=
    if [ "$BUILDER" = box ]; then
        loader=$ROOTFS/usr/lib/x86_64-linux-gnu/libvulkan.so.1
    else
        loader=$(ldconfig -p | awk '/libvulkan\.so\.1 /{print $NF; exit}')
    fi
    if [ -n "$loader" ] && [ -e "$loader" ]; then
        cp -L "$loader" "$APPDIR/usr/lib/vulkan/libvulkan.so.1"
        chmod 0644 "$APPDIR/usr/lib/vulkan/libvulkan.so.1"
        info "vulkan loader fallback: $loader"
    else
        rmdir "$APPDIR/usr/lib/vulkan" 2>/dev/null || true
        warn "no libvulkan.so.1 found to bundle as a fallback"
    fi

    # --- strip + rpath -------------------------------------------------------
    log "stripping our binaries and setting RUNPATH"
    run strip --strip-unneeded "$APPDIR/usr/bin/voyagersp" \
                              "$APPDIR/usr/lib/libefgame.so" \
                              "$APPDIR/usr/lib/libefui.so"
    # $ORIGIN, so the engine and its dlopen()ed modules find usr/lib next to
    # themselves even when something starts them without AppRun's
    # LD_LIBRARY_PATH (a file manager, the desktop file, an extracted AppDir).
    if run sh -c 'command -v patchelf >/dev/null 2>&1'; then
        run patchelf --set-rpath '$ORIGIN/../lib:$ORIGIN' "$APPDIR/usr/bin/voyagersp"
        run patchelf --set-rpath '$ORIGIN' "$APPDIR/usr/lib/libefgame.so" "$APPDIR/usr/lib/libefui.so"
    elif [ "$BUILDER" = box ]; then
        die "patchelf is missing from the build box — re-run setup-buildbox.sh"
    else
        # Host fallback only: nothing is installed on the host by this script.
        # AppRun's LD_LIBRARY_PATH still makes the image work.
        warn "patchelf is not installed on this host — RUNPATH left unset"
        warn "the AppImage will rely on AppRun's LD_LIBRARY_PATH (works, not relocatable-bare)"
    fi
    info "$(du -sh "$APPDIR" | cut -f1) staged, $(find "$APPDIR" -type f | wc -l) files"
}

# --------------------------------------------------------------- verification
max_symbol_version() {   # $1 file, $2 prefix (GLIBC | GLIBCXX); empty when none
    run objdump -T "$1" 2>/dev/null | grep -o "$2_[0-9.]*" | sort -V | tail -1 || true
}

verify_static() {
    log "static verification (nothing is executed)"
    local f g c r
    printf '   %-24s %-12s %-12s %s\n' file GLIBC GLIBCXX RUNPATH

    for f in usr/bin/voyagersp usr/lib/libefgame.so usr/lib/libefui.so; do
        g=$(max_symbol_version "$APPDIR/$f" GLIBC)
        c=$(max_symbol_version "$APPDIR/$f" GLIBCXX)
        r=$(run readelf -d "$APPDIR/$f" 2>/dev/null | awk '/RUNPATH|RPATH/{sub(/.*\[/,"");sub(/\].*/,"");printf "%s ", $0}')
        printf '   %-24s %-12s %-12s %s\n' "$(basename "$f")" "${g:-none}" "${c:-none}" "${r:-<none>}"
    done

    # The image's floor is the maximum over *everything* it carries, including
    # the bundled third-party libraries.
    local worst_glibc=0 worst_cxx=0 v elf
    while IFS= read -r elf; do
        v=$(max_symbol_version "$elf" GLIBC)
        if [ -n "$v" ]; then
            v=${v#GLIBC_}
            ver_gt "$v" "$worst_glibc" && worst_glibc=$v
        fi
        v=$(max_symbol_version "$elf" GLIBCXX)
        if [ -n "$v" ]; then
            v=${v#GLIBCXX_}
            ver_gt "$v" "$worst_cxx" && worst_cxx=$v
        fi
    done < <(find "$APPDIR/usr" -type f \( -name 'voyagersp' -o -name '*.so' -o -name '*.so.*' \) | sort)

    GLIBC_FLOOR=$worst_glibc
    CXX_FLOOR=$worst_cxx
    printf '   %-24s %s\n' "max GLIBC required" "GLIBC_$GLIBC_FLOOR"
    printf '   %-24s %s\n' "max GLIBCXX required" "GLIBCXX_$CXX_FLOOR"
    case $GLIBC_FLOOR in
        2.3[0-9]|2.2*) : ;;
        *) warn "the glibc floor is GLIBC_$GLIBC_FLOOR — build in the box (setup-buildbox.sh) for a 2.35 floor" ;;
    esac

    # Relocatable rpath on everything we own?
    for f in usr/bin/voyagersp usr/lib/libefgame.so usr/lib/libefui.so; do
        run readelf -d "$APPDIR/$f" | grep -q 'RUNPATH\|RPATH' \
            || warn "no RUNPATH in $f (relies on AppRun's LD_LIBRARY_PATH)"
    done

    # No host build path may leak into the shipped binaries.
    local leaks
    leaks=$(run sh -c "grep -c '$HOME' '$APPDIR/usr/bin/voyagersp' '$APPDIR/usr/lib/libefgame.so' 2>/dev/null | awk -F: '{s+=\$2} END {print s+0}'")
    [ "${leaks:-0}" = "0" ] || warn "$leaks occurrence(s) of the build home path ($HOME) inside the binaries"

    # Closure completeness: every NEEDED of every staged ELF must resolve from the
    # staged tree + the host-provided set (as seen by the build rootfs).
    local missing=0 notfound
    notfound=$(run env LD_LIBRARY_PATH="$APPDIR/usr/lib" sh -c \
        "for f in '$APPDIR/usr/bin/voyagersp' '$APPDIR/usr/lib/'*.so*; do
             ldd \"\$f\" 2>/dev/null | awk '/not found/{print \$1}'
         done | sort -u") || true
    if [ -n "$notfound" ]; then
        warn "libraries not resolvable in the staging environment:"
        printf '     %s\n' $notfound >&2
        missing=$(printf '%s\n' "$notfound" | wc -l)
    else
        info "closure: every NEEDED library resolves"
    fi

    # ldd -r: undefined symbols must be resolvable from the staged tree. The game
    # modules reach the engine through the game_import_t / ui_import_t tables, not
    # through dynamic symbols, so they are expected to be clean too.
    local undefined
    undefined=$(run env LD_LIBRARY_PATH="$APPDIR/usr/lib" sh -c \
        "ldd -r '$APPDIR/usr/bin/voyagersp' 2>&1 | grep -c 'undefined symbol'" || true)
    info "ldd -r voyagersp: ${undefined:-0} undefined symbol(s)"
    for f in libefgame.so libefui.so; do
        undefined=$(run env LD_LIBRARY_PATH="$APPDIR/usr/lib" sh -c \
            "ldd -r '$APPDIR/usr/lib/$f' 2>&1 | grep -c 'undefined symbol'" || true)
        info "ldd -r $f: ${undefined:-0} undefined symbol(s)"
    done

    # AppRun must be a valid POSIX shell script and executable.
    run sh -n "$APPDIR/AppRun" || die "AppRun has a syntax error"
    [ -x "$APPDIR/AppRun" ] || die "AppRun is not executable"

    if command -v desktop-file-validate >/dev/null 2>&1; then
        desktop-file-validate "$APPDIR/$APP_ID.desktop" && info "desktop file: valid"
    fi
    if command -v appstreamcli >/dev/null 2>&1; then
        # validate-tree is exactly what appimagetool runs on the AppDir (desktop
        # file + metadata + icon together), so catching it here fails early.
        if appstreamcli validate-tree "$APPDIR" > "$LOGS/appstream.log" 2>&1; then
            info "AppStream metadata: valid (appstreamcli validate-tree)"
        else
            cat "$LOGS/appstream.log" >&2
            die "appstreamcli validate-tree failed — appimagetool would refuse this AppDir"
        fi
    fi

    [ "$missing" -eq 0 ] || warn "the AppImage may not start on a host lacking the libraries listed above"
    return 0
}

# ------------------------------------------------------------------- AppImage
make_appimage() {
    log "appimagetool $APPIMAGETOOL_VERSION (zstd, pinned runtime)"
    # APPIMAGE_EXTRACT_AND_RUN: appimagetool is itself an AppImage; this lets it
    # run without FUSE (containers, CI). It does not run our payload.
    ARCH=x86_64 VERSION="$VERSION" APPIMAGE_EXTRACT_AND_RUN=1 \
    "$TOOLS/appimagetool-x86_64.AppImage" \
        --runtime-file "$TOOLS/runtime-x86_64" \
        --comp zstd \
        --mksquashfs-opt -Xcompression-level --mksquashfs-opt 19 \
        --mksquashfs-opt -all-time --mksquashfs-opt "$EPOCH" \
        "$APPDIR" "$APPIMAGE" > "$LOGS/appimagetool.log" 2>&1 \
        || { tail -30 "$LOGS/appimagetool.log" >&2; die "appimagetool failed (logs/appimagetool.log)"; }
    grep -iE 'warn|appstream' "$LOGS/appimagetool.log" | sed 's/^/   /' || true
    info "$APPIMAGE_NAME: $(stat -c%s "$APPIMAGE") bytes"
}

verify_appimage() {
    log "verifying the image without running it"
    # --appimage-extract executes only the runtime's extraction path: AppRun (the
    # game) is not executed by this pipeline.
    local tmp=$BUILD/verify-extract diff_out rt_len
    rm -rf "$tmp"
    mkdir -p "$tmp"
    ( cd "$tmp" && "$APPIMAGE" --appimage-extract > "$LOGS/appimage-extract.log" 2>&1 ) \
        || { tail -20 "$LOGS/appimage-extract.log" >&2; die "the AppImage could not be extracted (broken image)"; }

    if diff_out=$(diff -r "$APPDIR" "$tmp/squashfs-root" 2>&1); then
        info "payload round-trip: squashfs-root is identical to the staged AppDir"
    else
        printf '%s\n' "$diff_out" | head -20 >&2
        die "the extracted payload differs from the staged AppDir"
    fi
    rm -rf "$tmp"

    # type-2 magic + squashfs superblock, and the runtime must be the pinned one.
    python3 - "$APPIMAGE" "$(stat -c%s "$TOOLS/runtime-x86_64")" <<'PY'
import sys
path, rt_len = sys.argv[1], int(sys.argv[2])
d = open(path, 'rb').read(1 << 20)
assert d[:4] == b'\x7fELF', 'not an ELF file'
assert d[8:11] == b'AI\x02', 'not an AppImage type 2'
# "hsqs" also occurs inside the runtime's own code, so look past the runtime.
i = d.find(b'hsqs')
while i != -1 and i < rt_len:
    i = d.find(b'hsqs', i + 1)
assert i > 0, 'no squashfs superblock after the runtime'
print(f'   AppImage type 2: runtime {rt_len} bytes, squashfs superblock at offset {i}')
PY
    # The embedded runtime must be the pinned one. appimagetool writes the
    # payload's MD5 into the runtime's tail, so byte identity does not hold; the
    # ELF build ID is the identity of the runtime binary itself.
    local rid pid
    rid=$(readelf -n "$APPIMAGE" 2>/dev/null | awk '/Build ID/{print $3; exit}')
    pid=$(readelf -n "$TOOLS/runtime-x86_64" 2>/dev/null | awk '/Build ID/{print $3; exit}')
    if [ -n "$rid" ] && [ "$rid" = "$pid" ]; then
        info "embedded runtime: BuildID $rid = pinned $RUNTIME_RELEASE runtime"
    else
        die "the embedded runtime is not the pinned one (build id '$rid' vs '$pid')"
    fi
}

write_release_info() {
    log "recording the build"
    {
        printf 'VoyagerSP %s — AppImage build information\n' "$VERSION"
        printf 'packaged %s (SOURCE_DATE_EPOCH=%s)\n\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')" "$EPOCH"
        printf 'builder         : %s\n' "$BUILDER"
        if [ "$BUILDER" = box ]; then
            printf 'build rootfs    : Ubuntu %s base, glibc %s\n' "$UBUNTU_BASE_VERSION" "$UBUNTU_BASE_GLIBC"
            printf 'rootfs sha256   : %s\n' "$UBUNTU_BASE_SHA256"
        else
            printf 'build rootfs    : host (%s)\n' "$( . /etc/os-release; echo "$PRETTY_NAME" )"
        fi
        printf 'glibc floor     : GLIBC_%s (max symbol version required)\n' "$GLIBC_FLOOR"
        printf 'libstdc++ floor : GLIBCXX_%s\n' "$CXX_FLOOR"
        printf 'appimagetool    : %s  sha256 %s\n' "$APPIMAGETOOL_VERSION" "$APPIMAGETOOL_SHA256"
        printf 'runtime         : %s  sha256 %s  (GPG %s)\n' "$RUNTIME_RELEASE" "$RUNTIME_SHA256" "$RUNTIME_SIGNING_FPR"
        printf 'SDL2            : %s (vendored, built from EFAndroid-SP/app/jni/SDL)\n' "$SDL2_VERSION"
        printf 'FFmpeg          : %s minimal build (%s; bink demuxer + protocol=file only)\n' "$FFMPEG_VERSION" "$FFMPEG_CODECS"
        printf 'squashfs        : zstd level 19, all-time %s\n\n' "$EPOCH"
        printf 'inputs (snapshot):\n'
        for f in voyagersp libefgame.so libefui.so; do
            printf '  %-14s %10s  sha256 %s\n' "$f" "$(stat -c%s "$SNAP/$f")" "$(sha256_of "$SNAP/$f")"
        done
        printf '\nbundled libraries (%s):\n' "$(ls "$APPDIR/usr/lib" | grep -c .)"
        ls "$APPDIR/usr/lib" | grep -v '^vulkan$' | sed 's/^/  /'
        printf '  vulkan/libvulkan.so.1   (fallback only, not on LD_LIBRARY_PATH)\n'
        printf '\nleft to the host (%s, direct dependencies only):\n' "$(grep -c . "$BUILD/host-libs.txt" || true)"
        sed 's/^/  /' "$BUILD/host-libs.txt"
        printf '\nhost must also provide (loaded dynamically, not in ldd):\n'
        printf '  %s\n' \
            'glibc >= 2.35 and libstdc++ >= GLIBCXX_3.4.29' \
            'libvulkan.so.1 + a working Vulkan ICD/driver for the GPU' \
            'X11 client libraries (libX11, libXcursor, libXrandr, libXi, libXfixes, libXss) or XWayland' \
            'libasound2 or libpulse0 for sound (SDL loads them at run time)' \
            'libopenal.so.1 only if the user opts into +set s_useOpenAL 1' \
            'zenity or kdialog for the importer file picker (terminal prompt otherwise)'
        printf '\nthe AppImage was never executed by this pipeline; runtime testing is the user%s job\n' "'s"
    } > "$DIST/$APP_NAME-$VERSION-x86_64.build-info.txt"
    sha256sum "$APPIMAGE" | sed "s|$DIST/||" > "$APPIMAGE.sha256"
    info "dist/$APPIMAGE_NAME"
    info "dist/$APPIMAGE_NAME.sha256"
    info "dist/$APP_NAME-$VERSION-x86_64.build-info.txt"
}

# ----------------------------------------------------------------------- main
main() {
    log "VoyagerSP $VERSION"
    select_builder
    ensure_tools
    if [ "$DO_BUILD" -eq 1 ]; then
        build_all
    else
        info "--no-build: using the existing $RELEASE"
        [ -f "$RELEASE/voyagersp" ] || die "no build to package at $RELEASE (run without --no-build)"
    fi
    snapshot_outputs
    stage_appdir
    verify_static
    make_appimage
    verify_appimage
    write_release_info

    log "done"
    printf '   %-14s %s\n' "AppImage" "$APPIMAGE"
    printf '   %-14s %s\n' "size" "$(du -h "$APPIMAGE" | cut -f1)"
    printf '   %-14s %s\n' "sha256" "$(sha256_of "$APPIMAGE")"
    printf '   %-14s GLIBC_%s\n' "glibc floor" "$GLIBC_FLOOR"
    echo
    echo "   The image was NOT executed: this pipeline never launches the game."
    echo "   Runtime testing is the user's job — see README.md, \"Testing\"."
}

main
