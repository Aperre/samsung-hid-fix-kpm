# Samsung HID Fix

Fixes a freeze reported on some Samsung SM8550 devices when a USB HID gadget
is created through configfs and connected to a PC. Symptoms may include a
severely slowed/frozen UI and USB monitor, GPU fence, or workqueue errors.

The affected path is Samsung's `usb_f_ss_mon_gadget` monitor code. The fix
suppresses two functions:

- `usb_reset_notify`
- `make_suspend_current_event`

The optional `vbus_session_notify` hook was removed because it is not needed.

## Native KernelSU module (no KPM)

The `kernelsu-module/` directory contains a KernelSU package source and a
native Linux kernel module. It skips the two functions at entry using native
Kprobes and the ARM64 calling convention, rather than using the SukiSU/
KernelPatch KPM API.

**Build against the exact target device kernel tree and configuration**; there
is no universal prebuilt `.ko`. See
[`kernelsu-module/README.md`](kernelsu-module/README.md) for requirements,
build and installation instructions.

## Original KPM build

The original KPM source/build remains available at the repository root:

```sh
git submodule update --init --recursive
make TARGET_COMPILE=aarch64-linux-gnu-
```

Output: `build/samsung_hid_fix.kpm`.
