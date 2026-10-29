#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
KDIR="${1:-}"
CROSS_COMPILE="${2:-aarch64-linux-gnu-}"
OUTPUT_ZIP="${3:-"$SCRIPT_DIR/samsung-hid-fix-ksu.zip"}"

if [[ -z "$KDIR" || ! -f "$KDIR/Makefile" ]]; then
    echo "Usage: $0 /path/to/device/kernel [CROSS_COMPILE] [output.zip]" >&2
    echo "Optional: set KERNEL_OUT=/path/to/configured/kernel/output" >&2
    exit 2
fi

if [[ "$OUTPUT_ZIP" != /* ]]; then
    OUTPUT_ZIP="$PWD/$OUTPUT_ZIP"
fi

MAKE_ARGS=(-C "$KDIR" M="$SCRIPT_DIR" ARCH=arm64 CROSS_COMPILE="$CROSS_COMPILE")
if [[ -n "${KERNEL_OUT:-}" ]]; then
    MAKE_ARGS+=(O="$KERNEL_OUT")
fi

make "${MAKE_ARGS[@]}" modules

KO="$SCRIPT_DIR/samsung_hid_fix_lkm.ko"
if [[ ! -s "$KO" ]]; then
    echo "ERROR: build completed without producing $KO" >&2
    exit 1
fi

PKG_DIR="$(mktemp -d)"
trap 'rm -rf "$PKG_DIR"' EXIT
cp "$SCRIPT_DIR/module.prop" "$SCRIPT_DIR/service.sh" "$KO" "$PKG_DIR/"
mkdir -p "$(dirname -- "$OUTPUT_ZIP")"
(
    cd "$PKG_DIR"
    zip -q -r "$OUTPUT_ZIP" module.prop service.sh samsung_hid_fix_lkm.ko
)

echo "Created KernelSU package: $OUTPUT_ZIP"
echo "Built module: $KO"
