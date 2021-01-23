/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AMD Sensor Fusion Hub HID low-level driver interface
 *
 * Authors: Sandeep Singh <sandeep.singh@amd.com>
 *          Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 *          Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_HID_LL_DRV_H
#define AMD_SFH_HID_LL_DRV_H

#include <linux/hid.h>
#include <linux/pci.h>

#include "amd-sfh-pci.h"

/**
 * struct amd_sfh_hid_data - Per HID device driver data.
 * @sensor_idx:		The sensor index
 * @pci_dev:		The uderlying PCI device
 * @work:		Work buffer for device polling
 * @hid:		Backref to the hid device
 * @cpu_addr:		The DMA mapped CPU address
 * @dma_handle:		The DMA handle
 * @interval:		The sensor update interval
 */
struct amd_sfh_hid_data {
	enum sensor_idx sensor_idx;
	struct pci_dev *pci_dev;
	struct delayed_work work;
	struct hid_device *hid;
	u32 *cpu_addr;
	dma_addr_t dma_handle;
	unsigned int interval;
};

/* The low-level driver for AMD SFH HID devices */
extern struct hid_ll_driver amd_sfh_hid_ll_driver;

#endif
