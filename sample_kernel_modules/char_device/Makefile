obj-m += char_device.o
 
KDIR = /lib/modules/$(shell uname -r)/build

subdir-ccflags-y += -Werror
# subdir-ccflags-y += -g -O0
# dbg-$(CONFIG_ARM64) += debug_arm64.o
# obj-y += drivers/
# obj-m += $(dbg-y)
   
all:
	make -C $(KDIR) M=$(shell pwd) modules
	   
clean:
	make -C $(KDIR) M=$(shell pwd) clean
