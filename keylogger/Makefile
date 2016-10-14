obj-m += keylogger.o

all: module main

main: ioctl.c
	gcc ioctl.c -o main
	
module: keylogger.c keylogger.h
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f main
