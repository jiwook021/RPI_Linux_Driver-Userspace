KERNDIR=/lib/modules/`uname -r`/build
obj-m+=High_Resolution_Timer_servo.o
objs+=High_Resolution_Timer_servo.o
PWD=$(shell pwd)

default:
	make -C $(KERNDIR) M=$(PWD) modules

clean:
	make -C $(KERNDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.o