#KERNDIR=/usr/src/linux-headers-5.15.76-v7l+
KERNDIR=/lib/modules/`uname -r`/build
obj-m+=driver_led_pwm_2.o
objs+=driver_led_pwm_2.o
PWD=$(shell pwd)

default:
	make -C $(KERNDIR) M=$(PWD) modules

clean:
	make -C $(KERNDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.o
