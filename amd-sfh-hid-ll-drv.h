/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AMD Sensor Fusion Hub HID low-level driver interface
 *
 * Authors:	Sandeep Singh <sandeep.singh@amd.com>
 *		Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_HID_LL_DRV_H
#define AMD_SFH_HID_LL_DRV_H

#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "amd-sfh.h"

/**
 * struct amd_sfh_hid_data - Per HID device driver data.
 * @work:		Work buffer for device polling
 * @hid:		Backref to the hid device
 * @pci_dev:		Underlying PCI device
 * @sensor_idx:		Sensor index
 * @cpu_addr:		DMA mapped CPU address
 * @dma_handle:		DMA handle
 */
struct amd_sfh_hid_data {
	struct delayed_work work;
	struct hid_device *hid;
	struct pci_dev *pci_dev;
	enum sensor_idx sensor_idx;
	u32 *cpu_addr;
	dma_addr_t dma_handle;
};

/* The low-level driver for AMD SFH HID devices */
extern struct hid_ll_driver amd_sfh_hid_ll_driver;

#endif
