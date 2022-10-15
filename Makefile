obj-y		+= src/

PWD=$(shell pwd)
KHEADERS=$(shell uname -r)

default:
	make -C /lib/modules/$(KHEADERS)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KHEADERS)/build M=$(PWD) clean