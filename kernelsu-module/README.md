# Samsung HID Fix — native KernelSU module

This package reproduces the two active hooks in the repository's KPM without
loading a KPM. It is a regular KernelSU module that loads a native Linux kernel
module (`samsung_hid_fix_lkm.ko`) at late boot.

## What it changes

The native module installs Kprobes on these Samsung
`usb_f_ss_mon_gadget` functions:

- `usb_reset_notify`
- `make_suspend_current_event`

At function entry, it skips the original body and returns zero to the caller,
matching the behavior of the KPM's two hooks. The optional
`vbus_session_notify` hook is intentionally not included.

## Important compatibility requirement

The ZIP is **not universal**. The `.ko` must be built for the exact kernel
source/configuration and toolchain of the target device. A mismatch can be
rejected by the kernel or fail to load. The target kernel needs:

- `CONFIG_MODULES`
- `CONFIG_KPROBES`
- ARM64/AArch64
- both target functions present, discoverable by symbol name, and probeable

This repository cannot prebuild a single reliable `.ko` for every SM8550
kernel variant. Use the matching device kernel source and configured output
directory, including `Module.symvers` if the kernel uses modversions.

## Build an installable ZIP

From this directory, use the exact kernel tree/build output used for your
device. For a configured in-tree build tree:

```sh
make KDIR=/path/to/device/kernel CROSS_COMPILE=aarch64-linux-gnu-
```

The packaging script builds the native module and generates a KernelSU ZIP:

```sh
bash ./build-ksu-zip.sh /path/to/device/kernel aarch64-linux-gnu- ./samsung-hid-fix-ksu.zip
```

If your kernel source and configured output are separate, set
`KERNEL_OUT=/path/to/configured/output` when invoking the packaging script.

Flash the generated ZIP in KernelSU Manager and reboot. Review
`/data/adb/samsung-hid-fix.log` after boot and
`dmesg | grep samsung_hid_fix_lkm` for load/hook errors. To disable it, turn
off the module in KernelSU Manager and reboot.

## Limitations

This implementation uses native Linux Kprobes and the AArch64 calling
convention rather than KernelPatch's KPM hook API. It fails closed if either
probe cannot be registered. It has not been runtime-tested on your phone; test
only with a bootable recovery path available.
