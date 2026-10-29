# Samsung SS Monitor HID Freeze Fix

This KernelSU package uses `samsung_hid_fix_lkm.ko`, the native hook module
built from `native/samsung_hid_fix_lkm.c`. It hooks Samsung's
`usb_reset_notify()` and `make_suspend_current_event()` functions without
trying to load a second copy of `usb_f_ss_mon_gadget.ko`.

## GitHub Actions

The workflow `.github/workflows/build-ksu-module.yml` builds the native LKM
against the SM8550 kernel source and packages it into a KernelSU ZIP. Download
the ZIP and standalone LKM from the Actions run artifacts.

## Install and diagnose

Install the ZIP in KernelSU and reboot. The service waits up to 30 seconds for
`usb_f_ss_mon_gadget`, then loads `samsung_hid_fix_lkm.ko`. The service log
is `/data/adb/ss-mon-hid-fix.log`.

If the module still does not load, check that log and run `dmesg | grep -iE
'ksu|samsung_hid_fix|kprobe|unknown symbol|vermagic'`. A module built for a
different kernel build/configuration may fail to load.
