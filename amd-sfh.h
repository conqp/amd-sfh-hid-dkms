/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub PCIe driver interface
 *
 *  Authors: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *           Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_PCI_H
#define AMD_SFH_PCI_H

#include <linux/bits.h>
#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/types.h>

#define PCI_DEVICE_ID_AMD_SFH	0x15E4
#define AMD_SFH_MAX_HID_DEVICES	4

/**
 * Sensor Fusion Hub communication registers
 */
enum {
	/* SFH C2P Message Registers */
	AMD_C2P_MSG0 = 0x10500,		/* SFH command register */
	AMD_C2P_MSG1 = 0x10504,		/* SFH parameter register */
	AMD_C2P_MSG2 = 0x10508,		/* DRAM Address Lo / Data 0 */
	AMD_C2P_MSG3 = 0x1050c,		/* DRAM Address HI / Data 1 */
	AMD_C2P_MSG4 = 0x10510,		/* Data 2 */
	AMD_C2P_MSG5 = 0x10514,		/* Data 3 */
	AMD_C2P_MSG6 = 0x10518,		/* Data 4 */
	AMD_C2P_MSG7 = 0x1051c,		/* Data 5 */
	AMD_C2P_MSG8 = 0x10520,		/* Data 6 */
	AMD_C2P_MSG9 = 0x10524,		/* Data 7 */

	/* SFH P2C Message Registers */
	AMD_P2C_MSG0 = 0x10680,		/* Do not use */
	AMD_P2C_MSG1 = 0x10684,		/* I2C0 interrupt register */
	AMD_P2C_MSG2 = 0x10688,		/* I2C1 interrupt register */
	AMD_P2C_MSG3 = 0x1068C,		/* SFH sensor info */
	AMD_P2C_MSG_INTEN = 0x10690,	/* SFH interrupt gen register */
	AMD_P2C_MSG_INTSTS = 0x10694,	/* Interrupt status */
};

/**
 * The sensor indices on the AMD SFH device
 * @ACCEL_IDX:	Index of the accelerometer
 * @GYRO_IDX:	Index of the gyroscope
 * @MAG_IDX:	Index of the magnetometer
 * @ALS_IDX:	Index of the ambient light sensor
 */
enum sensor_idx {
	ACCEL_IDX = 0,
	GYRO_IDX,
	MAG_IDX,
	ALS_IDX = 19,
};

/**
 * Bit masks for sensors matching.
 * @ACCEL_MASK:	Bit mask of the accelerometer
 * @GYRO_MASK:	Bit mask of the gyroscope
 * @MAGNO_MASK:	Bit mask of the magnetometer
 * @ALS_MASK:	Bit mask of the ambient light sensor
 */
enum sensor_mask {
	ACCEL_MASK = BIT(ACCEL_IDX),
	GYRO_MASK = BIT(GYRO_IDX),
	MAGNO_MASK = BIT(MAG_IDX),
	ALS_MASK = BIT(ALS_IDX),
};

/**
 * SFH command IDs
 */
enum {
	AMD_SFH_CMD_NOOP = 0,
	AMD_SFH_CMD_ENABLE_SENSOR,
	AMD_SFH_CMD_DISABLE_SENSOR,
	AMD_SFH_CMD_DUMP_SENSOR_INFO,
	AMD_SFH_CMD_NUMBER_OF_SENSORS_DISCOVERED,
	AMD_SFH_CMD_WHOAMI_REGCHIPID,
	AMD_SFH_CMD_SET_DCD_DATA,
	AMD_SFH_CMD_GET_DCD_DATA,
	AMD_SFH_CMD_STOP_ALL_SENSORS,
	AMD_SFH_CMD_INVALID = 0xF,
};

/**
 * SFH command registers
 */
union amd_sfh_cmd {
	u32 ul;
	struct {
		u32 cmd_id : 8;
		u32 sensor_id : 8;
		u32 interval : 16;
	} s;
};

/**
 * SFH command parameters
 */
union amd_sfh_parm {
	u32 ul;
	struct {
		u32 buffer_layout : 2;
		u32 buffer_length : 6;
		u32 rsvd : 24;
	} s;
};

/**
 * struct amd_sfh_data - AMD SFH driver data
 * @mmio:		iommapped registers
 * @pci_dev:		The AMD SFH PCI device
 * @sensors:		The HID devices for the corresponding sensors
 */
struct amd_sfh_data {
	void __iomem *mmio;
	struct pci_dev *pci_dev;
	struct hid_device *sensors[AMD_SFH_MAX_HID_DEVICES];
};

/* SFH driver interface functions */
uint amd_sfh_get_sensor_mask(struct pci_dev *pci_dev);
uint amd_sfh_quirks_get_sensor_mask(void);
void amd_sfh_start_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx,
			  dma_addr_t dma_handle, unsigned int interval);
void amd_sfh_stop_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx);
int amd_sfh_client_init(struct amd_sfh_data *privdata);
int amd_sfh_client_deinit(struct amd_sfh_data *privdata);

#endif
