/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub platform interface
 *
 *  Authors: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *           Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_PLAT_H
#define AMD_SFH_PLAT_H

#include <linux/bits.h>
#include <linux/hid.h>
#include <linux/list.h>
#include <linux/pci.h>

#define ACCEL_MASK	BIT(ACCEL_IDX)
#define GYRO_MASK	BIT(GYRO_IDX)
#define MAGNO_MASK	BIT(MAG_IDX)
#define ALS_MASK	BIT(ALS_IDX)

/**
 * struct amd_sfh_plat_dev - Platform device data
 * @pci_dev:		The handled AMD SFH PCI device
 * @accel:		The HID device of the accelerometer
 * @gyro:		The HID device of the gyroscope
 * @magno:		The HID device of the magnetometer
 * @als:		The HID device of the ambient light sensor
 */
struct amd_sfh_plat_dev {
	struct pci_dev *pci_dev;
	struct hid_device *accel;
	struct hid_device *gyro;
	struct hid_device *magno;
	struct hid_device *als;
};

#endif
