#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o

obj-$(CONFIG_S3C2410_NAND_BOOT)	+= init.o
obj-y	+= cpu.o
obj-$(CONFIG_USE_IRQ)	+= interrupts.o

obj-$(CONFIG_EP93XX) += ep93xx/
obj-$(CONFIG_IMX) += imx/
obj-$(CONFIG_S3C24X0) += s3c24x0/

# some files can only build in ARM mode

ifdef CONFIG_SYS_THUMB_BUILD
CFLAGS_cpu.o := -marm
endif
