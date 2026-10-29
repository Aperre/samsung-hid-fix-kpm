# Samsung HID Fix — native KernelSU module

This package reproduces the fix in the repository's KPM without loading a KPM.
It is a regular KernelSU module that loads a small native Linux kernel module
(`samsung_hid_fix_lkm.ko`) at late boot.

## What it changes

The native module redirects these Samsung `usb_f_ss_mon_gadget` functions to
no-op replacements using the kernel's ftrace IPMODIFY interface:

- `usb_reset_notify`
- `make_suspend_current_event`

The optional `vbus_session_notify` hook is intentionally not included.

## Important compatibility requirement

The ZIP is **not universal**. The `.ko` must be built for the exact kernel
source/configuration and toolchain of the target device. A mismatch can be
rejected by the kernel or fail to load. The target kernel also needs:

- `CONFIG_MODULES`
- `CONFIG_FUNCTION_TRACER`
- `CONFIG_DYNAMIC_FTRACE`
- `CONFIG_KPROBES`
- register-saving/IPMODIFY support for ftrace on ARM64
- the target functions present and instrumented for ftrace

This repository cannot prebuild a single reliable `.ko` for every SM8550
kernel variant. Use the matching device kernel source and its configured
output directory (and `Module.symvers` when the kernel uses modversions).

## Build an installable ZIP

From this directory, use the exact kernel tree/build output used for your
device. For a configured in-tree build tree:

```sh
make KDIR=/path/to/device/kernel CROSS_COMPILE=aarch64-linux-gnu-
```

If your kernel source and configured output are separate, the kernel build
system's `O=` support may be needed; build with the exact matching configured
tree. The resulting `samsung_hid_fix_lkm.ko` should appear here.

Create the KernelSU ZIP:

```sh
./build-ksu-zip.sh /path/to/device/kernel aarch64-linux-gnu- ./samsung-hid-fix-ksu.zip
```

If your kernel needs a separate output directory, set `KERNEL_OUT=/path/to/out`
when invoking the script.

Flash the generated ZIP in KernelSU Manager and reboot. Review
`/data/adb/samsung-hid-fix.log` after boot, and use `dmesg | grep samsung_hid_fix_lkm`
to diagnose module load or hook errors. To turn it off, disable the module in
KernelSU Manager and reboot.

## Limitations

This implementation resolves function names with temporary kprobes and hooks
them using ftrace, rather than using KernelPatch's KPM hook API. It will fail
closed if either hook cannot be installed. It has not been runtime-tested on
your phone; test only with a bootable recovery path available.
