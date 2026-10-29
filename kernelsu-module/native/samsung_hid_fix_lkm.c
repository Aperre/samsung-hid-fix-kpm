// SPDX-License-Identifier: GPL-2.0-only
/*
 * Native-LKM implementation of Samsung HID Fix.
 *
 * No SukiSU/KernelPatch KPM API is used. At each target's entry, a kprobe
 * redirects execution to the caller return address and sets x0 to zero. This
 * skips the original function body, matching the KPM's skip_origin + ret=0.
 *
 * This implementation is intentionally ARM64-specific (the affected SM8550
 * devices use AArch64). Build against the exact target kernel tree/config.
 */

#include <linux/errno.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/ptrace.h>

#if !defined(CONFIG_KPROBES)
#error "Samsung HID Fix LKM requires CONFIG_KPROBES"
#endif

#if !defined(CONFIG_MODULES)
#error "Samsung HID Fix LKM requires CONFIG_MODULES"
#endif

#ifndef CONFIG_ARM64
#error "Samsung HID Fix LKM currently supports ARM64 kernels only"
#endif

#define HID_FIX_TAG "samsung_hid_fix_lkm: "

static int skip_target_function(struct kprobe *probe, struct pt_regs *regs)
{
	(void)probe;

	/*
	 * AArch64 procedure-call standard: x30 (regs[30]) holds the caller's
	 * return address at function entry; x0 (regs[0]) carries the return
	 * value. Returning nonzero tells Kprobes to skip the probed instruction.
	 */
	regs->regs[0] = 0;
	regs->pc = regs->regs[30];
	return 1;
}

struct hid_fix_probe {
	const char *name;
	struct kprobe probe;
	bool active;
};

static struct hid_fix_probe reset_probe = {
	.name = "usb_reset_notify",
};

static struct hid_fix_probe suspend_probe = {
	.name = "make_suspend_current_event",
};

static int install_probe(struct hid_fix_probe *target)
{
	int ret;

	target->probe.symbol_name = target->name;
	target->probe.pre_handler = skip_target_function;

	ret = register_kprobe(&target->probe);
	if (ret) {
		pr_err(HID_FIX_TAG "could not hook %s (kprobe error %d)\n",
		       target->name, ret);
		return ret;
	}

	target->active = true;
	pr_info(HID_FIX_TAG "hooked %s at %px\n", target->name,
		target->probe.addr);
	return 0;
}

static void remove_probe(struct hid_fix_probe *target)
{
	if (!target->active)
		return;

	unregister_kprobe(&target->probe);
	target->active = false;
	pr_info(HID_FIX_TAG "unhooked %s\n", target->name);
}

static int __init samsung_hid_fix_init(void)
{
	int ret;

	pr_info(HID_FIX_TAG "initializing native ARM64 kprobes (no KPM)\n");

	ret = install_probe(&reset_probe);
	if (ret)
		return ret;

	ret = install_probe(&suspend_probe);
	if (ret) {
		remove_probe(&reset_probe);
		return ret;
	}

	pr_info(HID_FIX_TAG "both reset/suspend paths are suppressed\n");
	return 0;
}

static void __exit samsung_hid_fix_exit(void)
{
	remove_probe(&suspend_probe);
	remove_probe(&reset_probe);
	pr_info(HID_FIX_TAG "hooks removed\n");
}

module_init(samsung_hid_fix_init);
module_exit(samsung_hid_fix_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ape");
MODULE_DESCRIPTION("Native kprobe replacement for Samsung USB HID freeze paths");
MODULE_VERSION("0.1.0");
