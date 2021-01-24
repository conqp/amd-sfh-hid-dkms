# SPDX-License-Identifier: GPL-2.0
#
# Makefile - AMD SFH HID drivers
# Copyright (c) 2020-2021, Advanced Micro Devices, Inc.
#
#
ccflags-m := -Werror
obj-$(CONFIG_AMD_SFH_HID) += amd-sfh.o
amd-sfh-objs += amd-sfh-hid-ll-drv.o
amd-sfh-objs += amd-sfh-hid-reports.o
amd-sfh-objs += amd-sfh-pci.o
amd-sfh-objs += amd-sfh-plat.o
amd-sfh-objs += amd-sfh-quirks.o
