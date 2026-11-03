#!/usr/bin/env bash
#
# play_voyagersp.sh — launch the Linux desktop build of Voyager Elite Force SP.
#
# One tree, two targets: EFAndroid-SP/app/jni/CMakeLists.txt builds the engine as
# ./voyagersp on Linux (libmain.so on Android) and pulls in ../../../efgame for the
# two game modules, so a single build produces:
#
#     <build>/Release/voyagersp
#     <build>/Release/libefgame.so     (game + cgame + icarus)
#     <build>/Release/libefui.so       (ui)
#
# Retail data handling lives INSIDE the engine now: on startup voyagersp validates
# <data-root>/baseEF/, and if the retail files are missing it runs the same
# import/picker flow the Android app uses (see sp/data_import.c). This script only
# resolves the data root and execs the binary — it no longer discovers the retail
# install, creates a run tree or symlinks paks.
#
# The data root is $VOYAGERSP_DATA_ROOT if set, else
# ${XDG_DATA_HOME:-$HOME/.local/share}/voyagersp. The engine derives fs_basepath
# and fs_homepath from it, so no +set is required.
#
# Usage:
#   scripts/play_voyagersp.sh                 # launch (builds only if not built yet)
#   scripts/play_voyagersp.sh --build         # rebuild, then launch
#   scripts/play_voyagersp.sh --no-build      # never build (fail if the binary is missing)
#   scripts/play_voyagersp.sh --map borg1     # boot straight into a map
#   scripts/play_voyagersp.sh --import        # force the retail-data import/picker
#   scripts/play_voyagersp.sh --fullscreen    # start fullscreen (default: windowed)
#   scripts/play_voyagersp.sh -- --help       # unknown args go to the engine verbatim
#
# Env overrides:
#   VOYAGERSP_DATA_ROOT  data root (paks, config, saves)   [engine-owned]
#   EFSP_BUILD_DIR       cmake build dir  (default <repo>/EFAndroid-SP/app/jni/build-linux)
#   EFSP_MAP             same as --map
#   EFSP_WRAPPER         command to run the engine under, e.g. "gdb --args" or "mangohud"
#
# Exit codes: the engine's own (the import path uses 0 ok, 2 cancelled,
# 3 invalid selection, 4 no picker available).

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BUILD_DIR="${EFSP_BUILD_DIR:-$ROOT/EFAndroid-SP/app/jni/build-linux}"
REL="$BUILD_DIR/Release"
SRC="$ROOT/EFAndroid-SP/app/jni"

# ---- data root: set it or respect an existing override ----
if [ -n "${VOYAGERSP_DATA_ROOT:-}" ]; then
    DATA_ROOT="$VOYAGERSP_DATA_ROOT"
else
    DATA_ROOT="${XDG_DATA_HOME:-$HOME/.local/share}/voyagersp"
fi
export VOYAGERSP_DATA_ROOT="$DATA_ROOT"

BUILD=-1          # -1 = decide (build only if the binary is missing)
MAP="${EFSP_MAP:-}"
ENGINE_ARGS=()

while [ $# -gt 0 ]; do
    case "$1" in
        --build)        BUILD=1 ;;
        --no-build)     BUILD=0 ;;
        --map)          [ $# -ge 2 ] || { echo "--map needs a map name" >&2; exit 2; }
                        MAP="$2"; shift ;;
        --map=*)        MAP="${1#*=}" ;;
        --data-dir)     [ $# -ge 2 ] || { echo "--data-dir needs a path" >&2; exit 2; }
                        DATA_ROOT="$2"; export VOYAGERSP_DATA_ROOT="$2"; shift ;;
        --data-dir=*)   DATA_ROOT="${1#*=}"; export VOYAGERSP_DATA_ROOT="${1#*=}" ;;
        --help|-h)
            sed -n '2,45p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        --)             shift; while [ $# -gt 0 ]; do ENGINE_ARGS+=("$1"); shift; done; break ;;
        *)              ENGINE_ARGS+=("$1") ;;
    esac
    shift
done

echo "repo     : $ROOT"
echo "build dir: $BUILD_DIR"
echo "data root: $DATA_ROOT"
if [ -n "${EFSP_BUILD_DIR:-}" ]; then
    echo "NOTE: EFSP_BUILD_DIR is set — this launch uses $BUILD_DIR" >&2
fi

# ---- build ----
if [ "$BUILD" -eq -1 ]; then
    if [ -x "$REL/voyagersp" ]; then BUILD=0; else BUILD=1; fi
fi

if [ "$BUILD" -eq 1 ]; then
    if [ ! -f "$BUILD_DIR/build.ninja" ]; then
        echo "configuring (cmake -S $SRC -B $BUILD_DIR -G Ninja -DEF_PLATFORM=linux -DCMAKE_BUILD_TYPE=Release) ..."
        cmake -S "$SRC" -B "$BUILD_DIR" -G Ninja -DEF_PLATFORM=linux \
              -DCMAKE_BUILD_TYPE=Release >/dev/null || {
            echo "cmake configure failed — see its output above" >&2; exit 1; }
    fi
    echo "building (cmake --build $BUILD_DIR) ..."
    if ! cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || echo 4)"; then
        echo "BUILD FAILED — refusing to launch a stale binary" >&2
        exit 1
    fi
else
    echo "not building (pass --build to rebuild first)"
fi

missing=0
for f in "$REL/voyagersp" "$REL/libefgame.so" "$REL/libefui.so"; do
    [ -f "$f" ] || { echo "missing $f" >&2; missing=1; }
done
if [ "$missing" -eq 1 ]; then
    echo "build outputs incomplete — run: $0 --build" >&2
    exit 1
fi
echo "binary   : $REL/voyagersp  ($(date -r "$REL/voyagersp" '+%Y-%m-%d %H:%M:%S'))"

# The engine dlopen()s libefgame.so/libefui.so by bare name: RUNPATH $ORIGIN
# already covers the Release dir, LD_LIBRARY_PATH is the belt-and-braces copy
# that also works when the binary runs through a wrapper.
export LD_LIBRARY_PATH="$REL${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

RUN_ARGS=()
[ -n "$MAP" ] && RUN_ARGS+=( +map "$MAP" )

echo "engine   : ${EFSP_WRAPPER:-} $REL/voyagersp ${RUN_ARGS[*]} ${ENGINE_ARGS[*]}"

cd "$REL" || exit 1
# exec, so the engine gets the terminal, signals and the real exit code.
exec ${EFSP_WRAPPER:-} ./voyagersp "${RUN_ARGS[@]}" "${ENGINE_ARGS[@]}"
