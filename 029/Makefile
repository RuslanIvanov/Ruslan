CURRENT=$(shell uname -r)
KDIR=/lib/modules/$(CURRENT)/build
PWD=$(shell pwd)
#DEST=/lib/modules/$(CURRENT)/misc
DEST=PWD
#TARGET=hello_mod
#obj-m:=$(TARGET).o
obj-m += hello_mod.o
obj-m += main_mod.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order *.ko
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
