KERNDIR=/lib/modules/`uname -r`/build
obj-m+=High_Resolution_Timer_pwm.o
objs+=High_Resolution_Timer_pwm.o
PWD=$(shell pwd)

default:
	make -C $(KERNDIR) M=$(PWD) modules

clean:
	make -C $(KERNDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.o