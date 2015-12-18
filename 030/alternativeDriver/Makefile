CURRENT=$(shell uname -r)
KDIR=/lib/modules/$(CURRENT)/build
PWD=$(shell pwd)
DEST=PWD

obj-m += kbuf.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order *.ko
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
