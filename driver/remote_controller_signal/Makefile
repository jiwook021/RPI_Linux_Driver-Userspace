#KERNDIR=/usr/src/linux-headers-5.15.76-v7l+
KERNDIR=/lib/modules/`uname -r`/build
obj-m+=remote_controller_signal.o
objs+=remote_controller_signal.o
PWD=$(shell pwd)

default:
	make -C $(KERNDIR) M=$(PWD) modules
clean:
	make -C $(KERNDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.o