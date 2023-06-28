KERNELDIR := /home/bo/linux/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
ROOTFS := /home/bo/linux/nfs/rootfs/lib/modules/4.1.15+/

CURRENT_PATH := $(shell pwd)

obj-m = icm20608reg.o
obj-app := $(patsubst %.o, %_app, ${obj-m})
obj-ko := $(patsubst %.o, %.ko, ${obj-m})

build: kernel_modules 

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	cp ${obj-ko} ${ROOTFS}
	arm-linux-gnueabihf-gcc ${obj-app}.c -o ${obj-app}
	cp ${obj-app} ${ROOTFS}
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	