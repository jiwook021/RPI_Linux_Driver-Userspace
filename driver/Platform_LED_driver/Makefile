obj-m += Platform_LED.o

ifdef ARCH
  KDIR = /home/heartha/lib/modules/5.10.65/build
else
  KDIR = /lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KDIR)  M=$(shell pwd) modules
	dtc -@ -I dts -O dtb -o test_overlay.dtbo test_overlay.dts
	sudo dtoverlay test_overlay.dtbo

clean:
	make -C $(KDIR)  M=$(shell pwd) clean