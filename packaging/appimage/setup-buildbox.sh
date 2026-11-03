#!/usr/bin/env bash
# setup-buildbox.sh — create the unprivileged Ubuntu 22.04 build box (glibc 2.35).
#
# Why: this host is Ubuntu 24.04 / glibc 2.39. A binary linked here needs
# GLIBC_2.38 for fmodf() (glibc 2.38 re-versioned it as the *default* fmodf), so
# it would refuse to start on Debian 12 (2.36), Ubuntu 22.04 (2.35) and older.
# Linking inside a 22.04 rootfs removes that symbol floor: the result needs 2.35.
#
# How: download Canonical's ubuntu-base 22.04.5 tarball, extract it under
# build/box/22.04/rootfs, then run apt *inside* a bwrap user namespace where our
# uid is 0 (see box-exec.sh). The host OS is never touched: no sudo, no packages
# installed outside this directory, no Docker, no loop mounts.
#
# Idempotent: re-running only does the work that is still missing. Everything it
# creates lives in build/ (gitignored).
#
# Usage: ./setup-buildbox.sh [--force-rootfs] [--force-sdl]

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
# shellcheck source=tools.lock
. "$HERE/tools.lock"

BOX_DIR=${VOYAGERSP_BOX_DIR:-$HERE/build/box/22.04}
ROOTFS=$BOX_DIR/rootfs
TARBALL=$BOX_DIR/ubuntu-base-22.04-base-amd64.tar.gz
FORCE_ROOTFS=0
FORCE_SDL=0
FORCE_FFMPEG=0

for arg in "$@"; do
    case $arg in
        --force-rootfs) FORCE_ROOTFS=1 ;;
        --force-sdl) FORCE_SDL=1 ;;
        --force-ffmpeg) FORCE_FFMPEG=1 ;;
        -h|--help) sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "setup-buildbox: unknown argument '$arg'" >&2; exit 2 ;;
    esac
done

log() { printf '\n== %s\n' "$*"; }
die() { printf 'setup-buildbox: %s\n' "$*" >&2; exit 1; }

command -v bwrap >/dev/null || die "bwrap (bubblewrap) is required"
bwrap --unshare-user --uid 0 --gid 0 --dev /dev --ro-bind / / true 2>/dev/null \
    || die "unprivileged user namespaces do not work on this kernel; the box cannot be used"

# ---------------------------------------------------------------- rootfs tarball
log "rootfs tarball ($UBUNTU_BASE_VERSION)"
mkdir -p "$BOX_DIR"
if [ ! -s "$TARBALL" ]; then
    curl -fL --retry 3 -o "$TARBALL.part" "$UBUNTU_BASE_URL"
    mv "$TARBALL.part" "$TARBALL"
fi
echo "$UBUNTU_BASE_SHA256  $TARBALL" | sha256sum -c --status \
    || die "checksum mismatch on $TARBALL (expected $UBUNTU_BASE_SHA256)"
echo "sha256 ok: $UBUNTU_BASE_SHA256"

# ------------------------------------------------------------------ extraction
if [ "$FORCE_ROOTFS" -eq 1 ] && [ -d "$ROOTFS" ]; then
    log "removing old rootfs (--force-rootfs)"
    chmod -R u+w "$ROOTFS" 2>/dev/null || true
    rm -rf "$ROOTFS"
fi
if [ ! -e "$ROOTFS/bin/bash" ]; then
    log "extracting rootfs -> $ROOTFS"
    rm -rf "$ROOTFS"
    mkdir -p "$ROOTFS"
    # --no-same-owner: everything ends up owned by us, which the user namespace
    # then maps to root:root inside. Device nodes are replaced by bwrap's --dev.
    tar -xzf "$TARBALL" -C "$ROOTFS" --no-same-owner --exclude='./dev/*' 2>/dev/null || true
    [ -e "$ROOTFS/bin/bash" ] || die "extraction failed"
fi

# ------------------------------------------------------------------- apt config
log "apt sources + sandbox override"
cat > "$ROOTFS/etc/apt/sources.list" <<'EOF'
deb http://archive.ubuntu.com/ubuntu jammy main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-updates main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-security main restricted universe multiverse
EOF
# apt normally drops to the _apt user for downloads; in this user namespace only
# uid 0 is mapped, so setgroups()/seteuid() fail. This is the documented headless
# container workaround, and it stays inside the box.
printf 'APT::Sandbox::User "root";\nAcquire::Retries "3";\n' \
    > "$ROOTFS/etc/apt/apt.conf.d/99voyagersp-sandbox"
mkdir -p "$ROOTFS/root" "$ROOTFS/opt"

# --------------------------------------------------------------------- packages
PKGS=(
    # toolchain
    build-essential cmake ninja-build pkg-config clang ca-certificates file
    # RUNPATH rewriting at packaging time ($ORIGIN, after stripping)
    patchelf
    # renderer / windowing
    libvulkan-dev
    # Bink movie playback (the engine links these; they are bundled into the AppImage)
    libavformat-dev libavcodec-dev libswscale-dev libavutil-dev
    # audio backends for the vendored SDL2
    libasound2-dev libpulse-dev
    # X11 + Wayland headers for the vendored SDL2 video backends
    libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxss-dev
    libxinerama-dev libxfixes-dev libxrender-dev
    libxkbcommon-dev libwayland-dev wayland-protocols libdecor-0-dev
)

need_install=0
"$HERE/box-exec.sh" -- bash -c 'for p in "$@"; do
        dpkg-query -W -f="\${Status}" "$p" 2>/dev/null | grep -q "install ok installed" || exit 1
    done' _ "${PKGS[@]}" || need_install=1

# Only gid 0 is mapped into the user namespace, so any chown() to a system group
# (staff, adm, ...) fails with EINVAL. fontconfig-config's postinst does exactly
# that — chown root:staff /usr/local/share/fonts — but only when it has to create
# the directory, and a failing postinst aborts the whole dpkg configure run.
# Creating the directory up front takes that branch out of play.
mkdir -p "$ROOTFS/usr/local/share/fonts"

if [ "$need_install" -eq 1 ]; then
    log "apt-get update + install (${#PKGS[@]} packages)"
    "$HERE/box-exec.sh" -- bash -c 'apt-get update -qq
        apt-get install -y -qq --no-install-recommends '"${PKGS[*]}" || true
fi

# Verify: dpkg leaves every dependent package unconfigured when one postinst
# fails. Retry once, then insist.
if ! "$HERE/box-exec.sh" -- bash -c 'for p in "$@"; do
        dpkg-query -W -f="\${Status}" "$p" 2>/dev/null | grep -q "install ok installed" || exit 1
    done' _ "${PKGS[@]}"; then
    log "some packages are not configured yet — apt-get -f install"
    "$HERE/box-exec.sh" -- apt-get -f install -y -qq || true
    "$HERE/box-exec.sh" -- bash -c 'for p in "$@"; do
            dpkg-query -W -f="\${Status}" "$p" 2>/dev/null | grep -q "install ok installed" || { echo "not configured: $p" >&2; exit 1; }
        done' _ "${PKGS[@]}" || die "the box is missing packages — inspect: box-exec.sh -- dpkg --configure -a"
else
    echo "all packages installed and configured"
fi

# ------------------------------------------------------------------------ SDL2
# The tree's vendored SDL (2.30.9, the same copy the APK ships) is built inside
# the box so the Linux build links a known SDL instead of the host distro's, and
# so the bundled libSDL2-2.0.so.0 has a glibc-2.35 floor.
SDLMARK=$ROOTFS/opt/sdl2/.voyagersp-built
if [ "$FORCE_SDL" -eq 1 ] || [ ! -e "$SDLMARK" ]; then
    log "building vendored SDL2 $SDL2_VERSION inside the box"
    rm -rf "$HERE/build/sdl2" "$ROOTFS/opt/sdl2"
    SDL_BUILD=$HERE/build/sdl2
    mkdir -p "$SDL_BUILD"
    # Kept deliberately small: X11 + Wayland, ALSA + PulseAudio. No GL/GLES (the
    # renderer is Vulkan), no KMSDRM, no hidapi/libusb, no JACK/PipeWire/sndio,
    # no IBus/DBus — every backend enabled here is a runtime dependency the
    # AppImage then has to satisfy.
    "$HERE/box-exec.sh" -- cmake -S "$REPO/EFAndroid-SP/app/jni/SDL" -B "$SDL_BUILD" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/sdl2 \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF -DSDL_INSTALL_TESTS=OFF \
        -DSDL_X11=ON -DSDL_WAYLAND=ON -DSDL_ALSA=ON -DSDL_PULSEAUDIO=ON \
        -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_VULKAN=ON \
        -DSDL_KMSDRM=OFF -DSDL_HIDAPI=OFF -DSDL_LIBUDEV=OFF \
        -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF -DSDL_DBUS=OFF \
        -DSDL_IBUS=OFF -DSDL_WERROR=OFF \
        > "$BOX_DIR/sdl-configure.log" 2>&1 \
        || { echo "SDL configure failed; see $BOX_DIR/sdl-configure.log" >&2; exit 1; }
    "$HERE/box-exec.sh" -- cmake --build "$SDL_BUILD" --target install -j"$(nproc)" \
        > "$BOX_DIR/sdl-build.log" 2>&1 \
        || { echo "SDL build failed; see $BOX_DIR/sdl-build.log" >&2; exit 1; }
    : > "$SDLMARK"
else
    echo "vendored SDL2 already built in the box"
fi

# --------------------------------------------------------------------- FFmpeg
# Why from source: the engine's Bink path (sp/bink_ff.c) uses the FFmpeg >= 5.1
# API (AVCodecContext.ch_layout); jammy ships 4.4, which only has
# channel_layout. Rather than depend on the engine growing a version guard for
# this box, the box builds the same FFmpeg the host has (6.1.1), pinned.
#
# The configuration is deliberately tiny: the Bink demuxer, its two audio
# decoders, binkvideo and swscale, everything else off — the same component set
# the Android build's static FFmpeg libraries carry (bink_ff.c is the only ffmpeg
# user in the tree). That also keeps libavformat's gnutls/zmq/srt/ssh/bluray
# closure out of the AppImage: with --disable-autodetect the four libraries
# depend on nothing but libc/libm/libpthread.
FFMPEG_SRC=$HERE/build/ffmpeg-src
FFMPEG_TARBALL=$BOX_DIR/ffmpeg-$FFMPEG_VERSION.tar.xz
FFMPEG_MARK=$ROOTFS/opt/ffmpeg/.voyagersp-built
if [ "$FORCE_FFMPEG" -eq 1 ] || [ ! -e "$FFMPEG_MARK" ]; then
    log "building FFmpeg $FFMPEG_VERSION (Bink only) inside the box"
    if [ ! -s "$FFMPEG_TARBALL" ]; then
        curl -fL --retry 3 -o "$FFMPEG_TARBALL.part" "$FFMPEG_URL" \
            || die "cannot download $FFMPEG_URL"
        mv "$FFMPEG_TARBALL.part" "$FFMPEG_TARBALL"
    fi
    echo "$FFMPEG_SHA256  $FFMPEG_TARBALL" | sha256sum -c --status \
        || die "checksum mismatch on $FFMPEG_TARBALL (expected $FFMPEG_SHA256)"
    echo "sha256 ok: $FFMPEG_SHA256"

    rm -rf "$FFMPEG_SRC" "$HERE/build/ffmpeg-$FFMPEG_VERSION" "$ROOTFS/opt/ffmpeg"
    tar -xf "$FFMPEG_TARBALL" -C "$HERE/build"
    mv "$HERE/build/ffmpeg-$FFMPEG_VERSION" "$FFMPEG_SRC"

    # Keep this list in the build log and in the README: it is the whole feature
    # set of the FFmpeg that ships inside the AppImage.
    "$HERE/box-exec.sh" -- sh -c "cd '$FFMPEG_SRC' && ./configure \
        --prefix=/opt/ffmpeg \
        --disable-everything \
        --disable-programs --disable-doc --disable-network --disable-avdevice \
        --disable-postproc --disable-avfilter --disable-filters \
        --disable-autodetect --disable-x86asm --disable-debug \
        --enable-shared --disable-static \
        --enable-demuxer=bink \
        --enable-decoder=$FFMPEG_CODECS \
        --enable-protocol=file \
        --enable-swscale --enable-swresample" \
        > "$BOX_DIR/ffmpeg-configure.log" 2>&1 \
        || { tail -25 "$BOX_DIR/ffmpeg-configure.log" >&2; die "FFmpeg configure failed"; }

    # Verify the component set is exactly what was asked for: a hidden default
    # (say, a parser or a second demuxer) would quietly widen the build.
    ff_count() { grep -c '&ff_' "$FFMPEG_SRC/$1" 2>/dev/null || true; }
    n_codecs=$(ff_count libavcodec/codec_list.c)
    n_demux=$(ff_count libavformat/demuxer_list.c)
    n_proto=$(ff_count libavformat/protocol_list.c)
    echo "components: $n_codecs codec(s), $n_demux demuxer(s), $n_proto protocol(s)"
    grep '&ff_' "$FFMPEG_SRC/libavcodec/codec_list.c" "$FFMPEG_SRC/libavformat/demuxer_list.c" \
        | sed 's/^/  /'
    [ "$n_codecs" = "3" ] || die "FFmpeg has $n_codecs codecs, expected 3 (bink, binkaudio_rdft, binkaudio_dct)"
    [ "$n_demux" = "1" ]  || die "FFmpeg has $n_demux demuxers, expected 1 (bink)"
    [ "$n_proto" = "1" ]  || die "FFmpeg has $n_proto protocols, expected 1 (file)"

    "$HERE/box-exec.sh" -- sh -c "cd '$FFMPEG_SRC' && make -j$(nproc) && make install" \
        > "$BOX_DIR/ffmpeg-build.log" 2>&1 \
        || { tail -25 "$BOX_DIR/ffmpeg-build.log" >&2; die "FFmpeg build failed"; }
    : > "$FFMPEG_MARK"
else
    echo "FFmpeg $FFMPEG_VERSION already built in the box"
fi

# ----------------------------------------------------------------------- report
log "box ready: $ROOTFS"
"$HERE/box-exec.sh" -- bash -c '
    printf "  %-24s %s\n" "os"            "$(. /etc/os-release; echo "$PRETTY_NAME")"
    printf "  %-24s %s\n" "glibc"         "$(ldd --version | head -1 | awk "{print \$NF}")"
    printf "  %-24s %s\n" "gcc"           "$(gcc -dumpfullversion)"
    printf "  %-24s %s\n" "clang"         "$(clang --version | head -1 | grep -oE "[0-9]+\.[0-9]+\.[0-9]+" | head -1)"
    printf "  %-24s %s\n" "cmake"         "$(cmake --version | head -1 | cut -d" " -f3)"
    printf "  %-24s %s\n" "ninja"         "$(ninja --version)"
    printf "  %-24s %s\n" "SDL2 (vendored)" "$(grep -hE "^#define SDL_(MAJOR_VERSION|MINOR_VERSION|PATCHLEVEL)" /opt/sdl2/include/SDL2/SDL_version.h | awk "{printf \"%s.\", \$3}" | sed "s/\\.\$//")"
    printf "  %-24s %s\n" "FFmpeg (built)" "$(PKG_CONFIG_PATH=/opt/ffmpeg/lib/pkgconfig pkg-config --modversion libavcodec libavformat libswscale libavutil 2>/dev/null | tr "\n" " ")"
    printf "  %-24s %s\n" "FFmpeg prefix" "/opt/ffmpeg"
'
echo
echo "next: ./build-appimage.sh   (it will use this box automatically)"
