#!/usr/bin/env bash
# box-exec.sh — run one command inside the unprivileged Ubuntu 22.04 build box.
#
# The box is a plain rootfs extracted under build/box/22.04/rootfs (no chroot
# privileges needed, no Docker, no sudo): bwrap(1) creates a user namespace, maps
# our uid to 0 inside it, and bind-mounts the rootfs as /. The repository is
# bind-mounted at the same absolute path, so build dirs written inside the box
# keep working outside it and CMake caches are not path-mismatched.
#
# Usage:  box-exec.sh [--] command [args...]
#         box-exec.sh --check          # exit 0 if the box is usable
#
# The box needs the rootfs, which setup-buildbox.sh creates.

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
BOX_DIR=${VOYAGERSP_BOX_DIR:-$HERE/build/box/22.04}
ROOTFS=$BOX_DIR/rootfs

die() { printf 'box-exec: %s\n' "$*" >&2; exit 1; }

check_userns() {
    command -v bwrap >/dev/null 2>&1 || die "bwrap not found (install bubblewrap)"
    bwrap --unshare-user --uid 0 --gid 0 --dev /dev --ro-bind / / true 2>/dev/null \
        || die "unprivileged user namespaces are not usable here; cannot enter the build box"
}

[ "${1:-}" = "--check" ] && { check_userns; echo "box-exec: userns ok"; exit 0; }
[ "${1:-}" = "--" ] && shift
[ $# -gt 0 ] || die "no command given"

[ -d "$ROOTFS" ] || die "no rootfs at $ROOTFS — run setup-buildbox.sh first"
check_userns

# A resolv.conf copy, because the host's is usually a symlink into /run, which the
# box does not have. Only used by apt during setup; builds never need the network.
RESOLV=$BOX_DIR/resolv.conf
if [ ! -s "$RESOLV" ]; then
    cp "$(readlink -f /etc/resolv.conf)" "$RESOLV" 2>/dev/null || cp /etc/resolv.conf "$RESOLV"
fi

# --clearenv: nothing from the host leaks in (LD_LIBRARY_PATH, CFLAGS, CC, ...).
# HOME=/root lives in the rootfs and is writable (userns maps our uid to 0).
exec bwrap \
    --bind "$ROOTFS" / \
    --dev /dev \
    --proc /proc \
    --tmpfs /run \
    --tmpfs /tmp \
    --ro-bind "$RESOLV" /etc/resolv.conf \
    --bind "$REPO" "$REPO" \
    --chdir "$PWD" \
    --unshare-user --uid 0 --gid 0 \
    --unshare-pid --unshare-ipc \
    --die-with-parent \
    --clearenv \
    --setenv HOME /root \
    --setenv PATH /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    --setenv LANG C.UTF-8 \
    --setenv LC_ALL C.UTF-8 \
    --setenv TERM dumb \
    --setenv DEBIAN_FRONTEND noninteractive \
    "$@"
