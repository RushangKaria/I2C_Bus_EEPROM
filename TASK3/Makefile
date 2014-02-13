obj-m = I2C-FLASH.o

all:
	make ARCH=arm CROSS_COMPILE=arm-angstrom-linux-gnueabi- -C /home/esp/Kernel_Source_Trees/Angstrom_Linux/kernel/kernel/ M=$(PWD) modules
	arm-angstrom-linux-gnueabi-gcc test.c -o i2c-test-app

clean:
	make -C /home/esp/Kernel_Source_Trees/Angstrom_Linux/kernel/kernel/ M=$(PWD) clean
	rm i2c-test-app
