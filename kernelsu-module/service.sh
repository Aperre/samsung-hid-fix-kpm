#!/system/bin/sh
MODDIR="${0%/*}"
KO="$MODDIR/samsung_hid_fix_lkm.ko"
LOG="/data/adb/ss-mon-hid-fix.log"

mkdir -p /data/adb 2>/dev/null
exec >>"$LOG" 2>&1
echo "[$(date)] Samsung HID freeze fix starting"

if [ ! -s "$KO" ]; then
    echo "ERROR: missing native hook module: $KO"
    exit 1
fi

# Wait briefly for Samsung's USB gadget monitor module to become available.
i=0
while [ "$i" -lt 30 ]; do
    if grep -q '^usb_f_ss_mon_gadget ' /proc/modules 2>/dev/null; then
        break
    fi
    i=$((i + 1))
    sleep 1
done

if ! grep -q '^usb_f_ss_mon_gadget ' /proc/modules 2>/dev/null; then
    echo "ERROR: usb_f_ss_mon_gadget is not loaded; hooks cannot be installed yet."
    exit 1
fi

if grep -q '^samsung_hid_fix_lkm ' /proc/modules 2>/dev/null; then
    echo "Native hook module is already loaded."
    exit 0
fi

if insmod "$KO"; then
    echo "Loaded samsung_hid_fix_lkm.ko"
else
    echo "ERROR: insmod failed. Check vermagic, symbol versions, KernelSU permissions, and dmesg."
    dmesg | tail -n 100
    exit 1
fi
