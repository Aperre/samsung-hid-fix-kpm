# Samsung HID Fix

Fixes a freeze reported on some Samsung SM8550 devices when a USB HID gadget
is created through configfs and connected to a PC. Symptoms may include a
severely slowed/frozen UI and USB monitor, GPU fence, or workqueue errors.

The affected path is Samsung's `usb_f_ss_mon_gadget` monitor code. The fix
suppresses two functions:

- `usb_reset_notify`
- `make_suspend_current_event`

## Native KernelSU module (no KPM)

The `kernelsu-module/` directory contains a KernelSU package source and a
native Linux kernel module. It skips the two functions at entry using native
Kprobes and the ARM64 calling convention, rather than using the SukiSU/
KernelPatch KPM API.

**Build against the exact target device kernel tree and configuration**; there
is no universal prebuilt `.ko`. See
[`kernelsu-module/README.md`](kernelsu-module/README.md) for requirements,
build and installation instructions.

## Build

```sh
git clone https://github.com/Aperre/samsung-hid-fix-kpm
cd samsung-hid-fix-kpm/
make TARGET_COMPILE=aarch64-linux-gnu-
```

Output: `build/samsung_hid_fix.kpm`.
