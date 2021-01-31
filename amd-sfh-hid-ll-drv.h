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
#include <linux/types.h>
#include <linux/workqueue.h>

#include "amd-sfh.h"

/**
 * struct amd_sfh_hid_data - Per HID device driver data.
 * @work:		Work buffer for device polling
 * @hid:		Backref to the hid device
 * @pci_dev:		The uderlying PCI device
 * @sensor_idx:		The sensor index
 * @cpu_addr:		The DMA mapped CPU address
 * @dma_handle:		The DMA handle
 * @descriptor_size:	Size of the descriptor buffer
 * @descriptor_buf:	Buffer for the report descriptor
 * @report_size:	Size of the input report buffer
 * @report_buf:		Buffer for the input report
 */
struct amd_sfh_hid_data {
	struct delayed_work work;
	struct hid_device *hid;
	struct pci_dev *pci_dev;
	enum sensor_idx sensor_idx;
	u32 *cpu_addr;
	dma_addr_t dma_handle;
	size_t report_size;
	u8 *report_buf;
};

/* The low-level driver for AMD SFH HID devices */
extern struct hid_ll_driver amd_sfh_hid_ll_driver;

#endif
