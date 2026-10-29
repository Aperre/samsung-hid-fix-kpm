// SPDX-License-Identifier: GPL-2.0-only
/*
 * Native-LKM implementation of Samsung HID Fix.
 *
 * This deliberately avoids SukiSU/KernelPatch KPM APIs. It redirects the two
 * target functions with ftrace IPMODIFY and resolves their addresses through
 * temporary kprobes, so it must be built against the exact target kernel tree.
 */

#include <linux/errno.h>
#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>

#if !defined(CONFIG_FUNCTION_TRACER) || !defined(CONFIG_DYNAMIC_FTRACE)
#error "Samsung HID Fix LKM requires CONFIG_FUNCTION_TRACER and CONFIG_DYNAMIC_FTRACE"
#endif

#if !defined(CONFIG_KPROBES)
#error "Samsung HID Fix LKM requires CONFIG_KPROBES for symbol resolution"
#endif

#define HID_FIX_TAG "samsung_hid_fix_lkm: "

struct hid_fix_hook {
	const char *name;
	unsigned long address;
	unsigned long replacement;
	struct ftrace_ops ops;
	bool filter_installed;
	bool active;
};

static noinline notrace unsigned long replace_usb_reset_notify(unsigned long arg0)
{
	(void)arg0;
	return 0;
}

static noinline notrace unsigned long replace_make_suspend_current_event(void)
{
	return 0;
}

static void hid_fix_ftrace_callback(unsigned long ip, unsigned long parent_ip,
				   struct ftrace_ops *ops,
				   struct ftrace_regs *fregs)
{
	struct hid_fix_hook *hook = ops->private;
	struct pt_regs *regs;

	(void)parent_ip;

	if (unlikely(!hook || ip != hook->address || !fregs))
		return;

	regs = ftrace_get_regs(fregs);
	if (likely(regs))
		regs->pc = hook->replacement;
}

static int resolve_symbol(const char *name, unsigned long *address)
{
	struct kprobe probe = {
		.symbol_name = name,
	};
	int ret;

	ret = register_kprobe(&probe);
	if (ret) {
		pr_err(HID_FIX_TAG "could not resolve '%s' (kprobe error %d)\n",
		       name, ret);
		return ret;
	}

	*address = (unsigned long)probe.addr;
	unregister_kprobe(&probe);

	if (!*address) {
		pr_err(HID_FIX_TAG "symbol '%s' resolved to a null address\n", name);
		return -ENOENT;
	}

	return 0;
}

static void remove_hook(struct hid_fix_hook *hook)
{
	if (hook->active) {
		unregister_ftrace_function(&hook->ops);
		hook->active = false;
	}

	if (hook->filter_installed) {
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		hook->filter_installed = false;
	}
}

static int install_hook(struct hid_fix_hook *hook)
{
	int ret;

	ret = resolve_symbol(hook->name, &hook->address);
	if (ret)
		return ret;

	hook->ops.func = hid_fix_ftrace_callback;
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;
	hook->ops.private = hook;

	ret = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	if (ret) {
		pr_err(HID_FIX_TAG "ftrace cannot filter '%s' at %px (error %d); "
		       "the function may not be instrumented by ftrace\n",
		       hook->name, (void *)hook->address, ret);
		return ret;
	}
	hook->filter_installed = true;

	ret = register_ftrace_function(&hook->ops);
	if (ret) {
		pr_err(HID_FIX_TAG "could not register ftrace hook for '%s' (error %d)\n",
		       hook->name, ret);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		hook->filter_installed = false;
		return ret;
	}

	hook->active = true;
	pr_info(HID_FIX_TAG "hooked %s at %px -> %px\n", hook->name,
		(void *)hook->address, (void *)hook->replacement);
	return 0;
}

static struct hid_fix_hook reset_hook = {
	.name = "usb_reset_notify",
	.replacement = (unsigned long)replace_usb_reset_notify,
};

static struct hid_fix_hook suspend_hook = {
	.name = "make_suspend_current_event",
	.replacement = (unsigned long)replace_make_suspend_current_event,
};

static int __init samsung_hid_fix_init(void)
{
	int ret;

	pr_info(HID_FIX_TAG "initializing native ftrace hooks (no KPM)\n");

	ret = install_hook(&reset_hook);
	if (ret)
		return ret;

	ret = install_hook(&suspend_hook);
	if (ret) {
		remove_hook(&reset_hook);
		return ret;
	}

	pr_info(HID_FIX_TAG "both reset/suspend paths are suppressed\n");
	return 0;
}

static void __exit samsung_hid_fix_exit(void)
{
	remove_hook(&suspend_hook);
	remove_hook(&reset_hook);
	pr_info(HID_FIX_TAG "hooks removed\n");
}

module_init(samsung_hid_fix_init);
module_exit(samsung_hid_fix_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ape");
MODULE_DESCRIPTION("Native ftrace replacement for Samsung USB HID freeze paths");
MODULE_VERSION("0.1.0");
