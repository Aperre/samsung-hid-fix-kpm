ifneq ($(KERNELRELEASE),)
obj-m += samsung_hid_fix_lkm.o
samsung_hid_fix_lkm-y := native/samsung_hid_fix_lkm.o
else
KDIR ?=
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-

.PHONY: all clean
all:
	@if [ -z "$(KDIR)" ]; then echo "Usage: make KDIR=/path/to/target/kernel [CROSS_COMPILE=aarch64-linux-gnu-]"; exit 2; fi
	$(MAKE) -C "$(KDIR)" M="$(CURDIR)" ARCH="$(ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" modules

clean:
	@if [ -n "$(KDIR)" ]; then $(MAKE) -C "$(KDIR)" M="$(CURDIR)" ARCH="$(ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" clean; else rm -f *.o *.ko *.mod *.mod.c Module.symvers modules.order; rm -f native/*.o native/.*.cmd .*.cmd; fi
endif
