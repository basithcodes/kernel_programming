obj-m := pcd_multi.o
ARCH=arm64
CROSS_COMPILER=aarch64-linux-gnu-
KERN_DIR=/home/basith/Coding/kernel_programming/raspberrypi/source/linux
HOST_KERN_DIR=/lib/modules/`uname -r`/build

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILER) -C $(KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
