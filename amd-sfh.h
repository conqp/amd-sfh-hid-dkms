/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub common interface
 *
 *  Authors:	Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_H
#define AMD_SFH_H

#include <linux/bits.h>
#include <linux/hid.h>
#include <linux/pci.h>

#define AMD_SFH_MAX_SENSORS	5

/**
 * The sensor indices on the AMD SFH device
 * @ACCEL_IDX:	Index of the accelerometer
 * @GYRO_IDX:	Index of the gyroscope
 * @MAG_IDX:	Index of the magnetometer
 * @LID_IDX:	Index of the lid switch
 * @ALS_IDX:	Index of the ambient light sensor
 */
enum sensor_idx {
	ACCEL_IDX = 0,
	GYRO_IDX,
	MAG_IDX,
	LID_IDX = 15,
	ALS_IDX = 19,
};

/**
 * Bit masks for sensors matching.
 * @ACCEL_MASK:	Bit mask of the accelerometer
 * @GYRO_MASK:	Bit mask of the gyroscope
 * @MAG_MASK:	Bit mask of the magnetometer
 * @LID_MASK:	Bit mask of the lid switch
 * @ALS_MASK:	Bit mask of the ambient light sensor
 */
enum sensor_mask {
	ACCEL_MASK = BIT(ACCEL_IDX),
	GYRO_MASK = BIT(GYRO_IDX),
	MAG_MASK = BIT(MAG_IDX),
	LID_MASK = BIT(LID_IDX),
	ALS_MASK = BIT(ALS_IDX),
};

/**
 * struct amd_sfh_data - AMD SFH driver data
 * @mmio:		iommapped registers
 * @pci_dev:		The AMD SFH PCI device
 * @sensors:		The HID devices for the corresponding sensors
 * @version:		SFH device version
 */
struct amd_sfh_data {
	void __iomem *mmio;
	struct pci_dev *pci_dev;
	struct hid_device *sensors[AMD_SFH_MAX_SENSORS];
	u8 version;
};

#endif
