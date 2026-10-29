#!/system/bin/sh
# Load the replacement driver packaged by the GitHub Actions build.
MODDIR="${0%/*}"
KO="$MODDIR/usb_f_ss_mon_gadget.ko"
LOG="/data/adb/ss-mon-hid-fix.log"

mkdir -p /data/adb 2>/dev/null
exec >>"$LOG" 2>&1
echo "[$(date)] Samsung SS monitor HID fix starting"

if [ ! -s "$KO" ]; then
    echo "ERROR: replacement driver is missing: $KO"
    exit 1
fi

if grep -q '^usb_f_ss_mon_gadget ' /proc/modules 2>/dev/null; then
    echo "usb_f_ss_mon_gadget is already loaded; skipping."
    exit 0
fi

if insmod "$KO"; then
    echo "Loaded replacement usb_f_ss_mon_gadget.ko"
else
    echo "ERROR: insmod failed. Check kernel release, vermagic, modversions, and exported symbols."
    dmesg | tail -n 100
    exit 1
fi
