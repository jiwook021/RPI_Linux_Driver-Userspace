obj-m += main.o

ifdef ARCH
  KDIR = /home/heartha/modules/5.10.65/build
else
  KDIR = /lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KDIR)  M=$(shell pwd) modules

clean:
	make -C $(KDIR)  M=$(shell pwd) clean
