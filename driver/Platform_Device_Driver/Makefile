
obj-m += dt_probe.o

all: module dt
	echo Builded Device Tree Overlay and kernel module

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: test_overlay.dts
	dtc -@ -I dts -O dtb -o test_overlay.dtbo test_overlay.dts
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf test_overlay.dtbo