#!/system/bin/sh
# Samsung HID Fix - KernelSU late-start service.
# The native LKM must be compiled against the exact running kernel build.

MODDIR="${0%/*}"
KO="$MODDIR/samsung_hid_fix_lkm.ko"
LOG="/data/adb/samsung-hid-fix.log"

mkdir -p /data/adb 2>/dev/null
exec >>"$LOG" 2>&1

echo "[$(date)] Samsung HID Fix service starting"

if [ ! -f "$KO" ]; then
    echo "ERROR: $KO is missing. Build the module for this device kernel and install the ZIP again."
    exit 1
fi

if grep -q '^samsung_hid_fix_lkm ' /proc/modules 2>/dev/null; then
    echo "Native module is already loaded; skipping."
    exit 0
fi

# The target functions live in Samsung's usb_f_ss_mon_gadget module. Wait briefly
# for it to load before resolving the function symbols in the companion LKM.
i=0
while [ "$i" -lt 45 ] && [ ! -d /sys/module/usb_f_ss_mon_gadget ]; do
    sleep 1
    i=$((i + 1))
done

if [ ! -d /sys/module/usb_f_ss_mon_gadget ]; then
    echo "WARNING: /sys/module/usb_f_ss_mon_gadget did not appear after 45 seconds; attempting load anyway."
fi

load_with_insmod() {
    if command -v insmod >/dev/null 2>&1; then
        echo "Trying insmod $KO"
        insmod "$KO"
        return $?
    fi
    return 127
}

if load_with_insmod; then
    echo "Native module loaded successfully."
    exit 0
fi

# Some KernelSU builds expose module loading through ksud instead of PATH's insmod.
for KSUD in "$(command -v ksud 2>/dev/null)" /data/adb/ksu/bin/ksud /data/adb/ksud; do
    [ -n "$KSUD" ] && [ -x "$KSUD" ] || continue
    echo "Trying $KSUD insmod $KO"
    if "$KSUD" insmod "$KO"; then
        echo "Native module loaded successfully via ksud."
        exit 0
    fi
done

echo "ERROR: failed to load native module. Check exact-kernel build, vermagic, exported symbols, and kernel configuration."
exit 1
