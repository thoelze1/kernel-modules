obj-m += chardev.o

all: chardev main

main: ioctl.c
	gcc ioctl.c -o main

chardev: chardev.c chardev.h
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f main
