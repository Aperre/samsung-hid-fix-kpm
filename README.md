# Samsung HID Fix KPM

This KPM fixes a freeze that can happen on some Samsung SM8550 devices when a
USB HID gadget is created through configfs and connected to a PC.

The symptom is usually:

- the phone connects as a HID device
- after a few seconds the UI becomes extremely slow or freezes
- kernel logs show `usb_f_ss_mon_gadget`, `UDC busy`, `kgsl-3d0` fence errors,
  GPU CX timeouts, and eventually workqueue lockups

## Cause

The HID gadget itself is not the direct problem.

The bad path is Samsung's `usb_f_ss_mon_gadget` monitor code. During USB
reset/suspend handling it queues work through the Samsung USB monitor path, and
that can cascade into GPU/display stalls and global workqueue lockups.

## Fix

This module uses SukiSU KernelPatch KPM support to hook and suppress:

- `usb_reset_notify`
- `make_suspend_current_event`

Those functions are part of Samsung's `usb_f_ss_mon_gadget` module. Suppressing
them avoids the freeze while keeping the HID gadget usable.

The module also supports an optional `all` argument, which additionally hooks:

- `vbus_session_notify`

That is not needed for the known HID freeze fix. Use the default no-argument
load first.

## Build

From the SukiSU KernelPatch source tree:

```sh
git clone https://github.com/Aperre/samsung-hid-fix-kpm
cd samsung-hid-fix-kpm/
make TARGET_COMPILE=aarch64-linux-gnu-
```

Output:

```text
samsung_hid_fix.kpm
```