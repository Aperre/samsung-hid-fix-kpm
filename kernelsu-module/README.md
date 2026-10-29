# Samsung SS Monitor HID Freeze Fix

This KernelSU package replaces Samsung's `usb_f_ss_mon_gadget.ko` with a
build of the driver source in `driver/f_ss_mon_gadget.c`. The replacement
keeps the existing gadget implementation and exported symbols but makes
`usb_reset_notify()` and `make_suspend_current_event()` no-ops.

## GitHub Actions

The workflow `.github/workflows/build-ksu-module.yml` builds the SM8550
kernel tree, stages this driver as an in-tree gadget function, and packages
the resulting `usb_f_ss_mon_gadget.ko` into a KernelSU ZIP. Download the ZIP
and standalone module from the Actions run's artifacts.

The workflow currently uses
`Aperre/Samsung_kernel_sm8550-common-SUSFS` branch
`fix/anykernel3-susfs-kdp-ns` for the kernel build.

## Compatibility

A kernel module is not universal. It must match the target kernel's release,
configuration, symbol versions, and vendor ABI. A successful CI build does
not prove it can load on a different phone/kernel build. Install only on the
matching kernel build and keep a recovery path available.

The service logs to `/data/adb/ss-mon-hid-fix.log`.
