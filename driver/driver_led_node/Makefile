#=======================================
# Makefile for device module
#=======================================
KERVER := $(shell uname -r)
PWD := $(shell pwd)

CC := /usr/bin/gcc
#KDIR := /usr/src/linux-headers-5.15.32-v7l+
#KERNDIR=/usr/src/linux-headers-5.15.76-v7l+
KERNDIR=/lib/modules/`uname -r`/build

obj-m := driver_led_node.o

TEST_TARGET = led_test
TEST_OBJS = led_test.o
TEST_SRCS = led_test.c

build: 
	make -C $(KERNDIR) M=$(PWD) modules

$(TEST_TARGET) : $(TEST_OBJS)
	$(CC) -o $@ $(TEST_OBJS)

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -rf *.o *.ko *.mod.c .*.cmd *.order *.symvers .tmp*
	rm -f $(TEST_TARGET)