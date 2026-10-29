#!/usr/bin/env bash
set -euo pipefail

KO="${1:-}"
OUTPUT_ZIP="${2:-}"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ -z "$KO" || ! -s "$KO" || -z "$OUTPUT_ZIP" ]]; then
    echo "Usage: $0 /path/to/samsung_hid_fix_lkm.ko /path/to/output.zip" >&2
    exit 2
fi

PKG_DIR="$(mktemp -d)"
trap 'rm -rf "$PKG_DIR"' EXIT

install -m 0644 "$SCRIPT_DIR/module.prop" "$PKG_DIR/module.prop"
install -m 0755 "$SCRIPT_DIR/service.sh" "$PKG_DIR/service.sh"
install -m 0644 "$KO" "$PKG_DIR/samsung_hid_fix_lkm.ko"

mkdir -p "$(dirname -- "$OUTPUT_ZIP")"
(
    cd "$PKG_DIR"
    zip -q -r "$OUTPUT_ZIP" module.prop service.sh samsung_hid_fix_lkm.ko
)

echo "Created KernelSU ZIP: $OUTPUT_ZIP"
