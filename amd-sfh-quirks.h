/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub quirks interface
 *
 *  Authors: Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_QUIRKS_H
#define AMD_SFH_QUIRKS_H

#include <linux/pci.h>
#include <linux/types.h>

/**
 * Sensor mask quirks
 * @sensor_mask:	Sensor mask override
 */
struct amd_sfh_quirks {
	uint sensor_mask;
};

uint amd_sfh_quirks_get_sensor_mask(struct pci_dev *pci_dev);

#endif
