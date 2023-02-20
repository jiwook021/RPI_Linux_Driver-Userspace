obj-m += driver_ioctl_interrupt.o

ifdef ARCH
  KDIR = /home/heartha/lib/modules/5.10.65/build
else
  KDIR = /lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KDIR)  M=$(shell pwd) modules

clean:
	make -C $(KDIR)  M=$(shell pwd) clean