KERNELDIR := /home/bo/linux/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek

CURRENT_PATH := $(shell pwd)

obj-m := mutexLed.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean

# cp mutexLed.ko ~/linux/nfs/rootfs/lib/modules/4.1.15+/