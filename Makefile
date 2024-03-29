# SPDX-License-Identifier: GPL-2.0
#
# Makefile - AMD SFH HID drivers
# Copyright (c) 2020-2021, Advanced Micro Devices, Inc.
#
#
ccflags-m := -Werror
obj-$(CONFIG_AMD_SFH_HID) += amd-sfh.o
amd-sfh-objs += amd-sfh-client.o
amd-sfh-objs += amd-sfh-hid-ll-drv.o
amd-sfh-objs += amd-sfh-pci.o
amd-sfh-objs += amd-sfh-quirks.o
amd-sfh-objs += sensors/amd-sfh-accel.o
amd-sfh-objs += sensors/amd-sfh-als.o
amd-sfh-objs += sensors/amd-sfh-gyro.o
amd-sfh-objs += sensors/amd-sfh-lid.o
amd-sfh-objs += sensors/amd-sfh-mag.o
