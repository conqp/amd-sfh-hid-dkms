/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub PCIe driver interface
 *
 *  Authors:	Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_PCI_H
#define AMD_SFH_PCI_H

#include <linux/pci.h>
#include <linux/types.h>

#include "amd-sfh.h"

#define AMD_SFH_UPDATE_INTERVAL		200

/**
 * Sensor Fusion Hub communication registers
 */
enum amd_sfh_pci_registers {
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
 * SFH command IDs
 */
enum amd_sfh_cmd_ids {
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

uint amd_sfh_get_sensor_mask(struct pci_dev *pci_dev);
void amd_sfh_start_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx,
			  dma_addr_t dma_handle);
void amd_sfh_stop_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx);

#endif
