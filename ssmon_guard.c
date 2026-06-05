// SPDX-License-Identifier: GPL-2.0-or-later

#include <compiler.h>
#include <hook.h>
#include <kallsyms.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <linux/string.h>

KPM_NAME("samsung_hid_fix_kpm");
KPM_VERSION("0.1.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Ape");
KPM_DESCRIPTION("Fix Samsung HID gadget freezes by suppressing ss_mon reset/suspend paths");

static void *usb_reset_notify_addr;
static void *make_suspend_current_event_addr;
static void *vbus_session_notify_addr;
static int hook_vbus;

static int str_eq(const char *a, const char *b)
{
	if (!a || !b)
		return 0;
	return strcmp(a, b) == 0;
}

static void before_usb_reset_notify(hook_fargs1_t *args, void *udata)
{
	args->skip_origin = 1;
	args->ret = 0;
}

static void before_make_suspend_current_event(hook_fargs0_t *args, void *udata)
{
	args->skip_origin = 1;
	args->ret = 0;
}

static void before_vbus_session_notify(hook_fargs3_t *args, void *udata)
{
	args->skip_origin = 1;
	args->ret = 0;
}

static long ssmon_guard_init(const char *args, const char *event, void *__user reserved)
{
	hook_err_t err;

	hook_vbus = str_eq(args, "all") || str_eq(args, "vbus");

	usb_reset_notify_addr = (void *)kallsyms_lookup_name("usb_reset_notify");
	make_suspend_current_event_addr =
		(void *)kallsyms_lookup_name("make_suspend_current_event");
	vbus_session_notify_addr = (void *)kallsyms_lookup_name("vbus_session_notify");

	pr_info("ssmon_guard: init args=%s event=%s reset=%px suspend=%px vbus=%px hook_vbus=%d\n",
		args, event, usb_reset_notify_addr, make_suspend_current_event_addr,
		vbus_session_notify_addr, hook_vbus);

	if (usb_reset_notify_addr) {
		err = hook_wrap1(usb_reset_notify_addr, before_usb_reset_notify,
				 0, 0);
		pr_info("ssmon_guard: hook usb_reset_notify err=%d\n", err);
	} else {
		pr_warn("ssmon_guard: usb_reset_notify not found\n");
	}

	if (make_suspend_current_event_addr) {
		err = hook_wrap0(make_suspend_current_event_addr,
				 before_make_suspend_current_event, 0, 0);
		pr_info("ssmon_guard: hook make_suspend_current_event err=%d\n",
			err);
	} else {
		pr_warn("ssmon_guard: make_suspend_current_event not found\n");
	}

	if (hook_vbus) {
		if (vbus_session_notify_addr) {
			err = hook_wrap3(vbus_session_notify_addr,
					 before_vbus_session_notify, 0, 0);
			pr_info("ssmon_guard: hook vbus_session_notify err=%d\n",
				err);
		} else {
			pr_warn("ssmon_guard: vbus_session_notify not found\n");
		}
	}

	return 0;
}

static long ssmon_guard_control0(const char *args, char *__user out_msg, int outlen)
{
	pr_info("ssmon_guard: control args=%s\n", args);
	return 0;
}

static long ssmon_guard_exit(void *__user reserved)
{
	if (usb_reset_notify_addr)
		hook_unwrap(usb_reset_notify_addr, before_usb_reset_notify, 0);
	if (make_suspend_current_event_addr)
		hook_unwrap(make_suspend_current_event_addr,
			    before_make_suspend_current_event, 0);
	if (hook_vbus && vbus_session_notify_addr)
		hook_unwrap(vbus_session_notify_addr,
			    before_vbus_session_notify, 0);

	pr_info("ssmon_guard: exit\n");
	return 0;
}

KPM_INIT(ssmon_guard_init);
KPM_CTL0(ssmon_guard_control0);
KPM_EXIT(ssmon_guard_exit);
