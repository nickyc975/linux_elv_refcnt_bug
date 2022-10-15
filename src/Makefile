# SPDX-License-Identifier: GPL-2.0

KHEADERS=$(shell uname -r)

ccflags-y				+= -I$(src)
ccflags-y               += -I/lib/modules/$(KHEADERS)/build/block

obj-m                   += dummy-iosched.o
obj-m	                += null_blk.o

null_blk-objs			:= main.o
null_blk-objs 	        += trace.o
null_blk-objs           += zoned.o