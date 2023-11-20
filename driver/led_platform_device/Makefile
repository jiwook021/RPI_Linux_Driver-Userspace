KBUILD_EXTRA_SYMBOLS :=
 
obj-m := led_platform_device.o
 
default:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean