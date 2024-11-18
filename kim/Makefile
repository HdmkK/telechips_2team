obj-m := test.o

KERNELDIR := /home/vboxuser/topst/build-main/workspace/sources/linux-telechips/
PWD := $(shell pwd)

CROSS_COMPILE ?= aarch64-linux-gnu-


default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) clean