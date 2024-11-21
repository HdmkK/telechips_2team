obj-m := Simple_motor.o

KERNELDIR := /home/user/Desktop/topst/build/tcc8050-main/tmp/work-shared/tcc8050-main/kernel-source/
PWD := $(shell pwd)

CROSS_COMPILE ?= aarch64-linux-gnu-


default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) clean
