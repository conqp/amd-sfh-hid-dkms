// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub PCIe driver
 *
 * Authors:	Shyam Sundar S K <Shyam-sundar.S-k@amd.com>
 *		Nehal Bakulchandra Shah <Nehal-bakulchandra.Shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/io-64-nonatomic-lo-hi.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/types.h>

#include "amd-sfh.h"
#include "amd-sfh-client.h"
#include "amd-sfh-pci.h"
#include "amd-sfh-quirks.h"

#define DRIVER_NAME		"amd_sfh"
#define PCI_DEVICE_ID_AMD_SFH	0x15E4

/* Module parameters */
static uint sensor_mask_override;
module_param_named(sensor_mask, sensor_mask_override, uint, 0644);
MODULE_PARM_DESC(sensor_mask, "override the sensors bitmask");

/**
 * amd_sfh_get_sensor_mask - Returns the sensors mask.
 * @pci_dev:	The Sensor Fusion Hub PCI device
 *
 * Returns an integer representing the bitmask to match
 * the sensors connected to the Sensor Fusion Hub.
 */
uint amd_sfh_get_sensor_mask(struct pci_dev *pci_dev)
{
	struct amd_sfh_data *privdata;
	struct amd_sfh_quirks *quirks;
	uint sensor_mask;

	privdata = pci_get_drvdata(pci_dev);

	/* Read bit-shifted sensor mask from P2C register */
	sensor_mask = readl(privdata->mmio + AMD_P2C_MSG3) >> 4;
	if (!sensor_mask)
		pci_err(pci_dev, "[Firmware Bug]: No sensors marked active!\n");

	if (sensor_mask_override)
		return sensor_mask_override;

	quirks = amd_sfh_get_quirks();
	if (quirks)
		return quirks->sensor_mask;

	return sensor_mask;
}

/**
 * amd_sfh_get_version - Returns the hardware version.
 * @privdata:	AMD SFH driver data
 *
 * Returns the hardware version ID.
 */
static u8 _amd_sfh_get_version(struct amd_sfh_data *privdata)
{
	return readl(privdata->mmio + AMD_P2C_MSG3) & GENMASK(3, 0);
}

/**
 * amd_sfh_get_version - Returns the hardware version.
 * @pci_dev:	Sensor Fusion Hub PCI device
 *
 * Returns the hardware version ID.
 */
u8 amd_sfh_get_version(struct pci_dev *pci_dev)
{
	return _amd_sfh_get_version(pci_get_drvdata(pci_dev));
}

/**
 * amd_sfh_get_illuminance - Returns the illumination value.
 * @pci_dev:	Sensor Fusion Hub PCI device
 */
int amd_sfh_get_illuminance(struct pci_dev, *pci_dev)
{
	struct amd_sfh_data *privdata = pci_get_drvdata(pci_dev);
	return (int)readl(privdata->mmio + AMD_C2P_MSG5);
}

/**
 * amd_sfh_start_sensor - Starts the respective sensor.
 * @pci_dev:	Sensor Fusion Hub PCI device
 * @sensor_idx:	Sensor index
 * @dma_handle:	DMA handle
 */
void amd_sfh_start_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx,
			  dma_addr_t dma_handle)
{
	struct amd_sfh_data *privdata;
	union amd_sfh_parm parm;
	union amd_sfh_cmd cmd;

	privdata = pci_get_drvdata(pci_dev);

	cmd.ul = 0;

	switch (_amd_sfh_get_version(privdata)) {
	case AMD_SFH_HWID_V2:
		cmd.cmd_v2.cmd_id = AMD_SFH_CMD_ENABLE_SENSOR;
		cmd.cmd_v2.period = AMD_SFH_UPDATE_INTERVAL;
		cmd.cmd_v2.sensor_id = sensor_idx;
		cmd.cmd_v2.length = 16;

		if (sensor_idx == ALS_IDX)
			cmd.cmd_v2.mem_type = AMD_SFH_USE_C2P_REG;

		break;
	default:
		cmd.cmd_v1.cmd_id = AMD_SFH_CMD_ENABLE_SENSOR;
		cmd.cmd_v1.interval = AMD_SFH_UPDATE_INTERVAL;
		cmd.cmd_v1.sensor_id = sensor_idx;
		break;
	}

	parm.ul = 0;
	parm.s.buffer_layout = 1;
	parm.s.buffer_length = 16;

	writeq(dma_handle, privdata->mmio + AMD_C2P_MSG2);
	writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
	writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);
}

/**
 * amd_sfh_stop_sensor - Stops the respective sensor.
 * @pci_dev:	Sensor Fusion Hub PCI device
 * @sensor_idx:	Sensors index
 */
void amd_sfh_stop_sensor(struct pci_dev *pci_dev, enum sensor_idx sensor_idx)
{
	struct amd_sfh_data *privdata;
	union amd_sfh_parm parm;
	union amd_sfh_cmd cmd;

	privdata = pci_get_drvdata(pci_dev);

	cmd.ul = 0;

	switch (_amd_sfh_get_version(privdata)) {
	case AMD_SFH_HWID_V2:
		cmd.cmd_v2.cmd_id = AMD_SFH_CMD_DISABLE_SENSOR;
		cmd.cmd_v2.period = 0;
		cmd.cmd_v2.sensor_id = sensor_idx;
		cmd.cmd_v2.length  = 16;
		break;
	default:
		cmd.cmd_v1.cmd_id = AMD_SFH_CMD_DISABLE_SENSOR;
		cmd.cmd_v1.interval = 0;
		cmd.cmd_v1.sensor_id = sensor_idx;
		break;
	}

	parm.ul = 0;

	writeq(0x0, privdata->mmio + AMD_C2P_MSG2);
	writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
	writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);
}

static void amd_sfh_stop_all_sensors(struct amd_sfh_data *privdata)
{
	union amd_sfh_parm parm;
	union amd_sfh_cmd cmd;

	cmd.ul = 0;

	switch (_amd_sfh_get_version(privdata)) {
	case AMD_SFH_HWID_V2:
		cmd.cmd_v2.cmd_id = AMD_SFH_CMD_STOP_ALL_SENSORS;
		cmd.cmd_v2.period = 0;
		cmd.cmd_v2.sensor_id = 0;
		break;
	default:
		cmd.cmd_v1.cmd_id = AMD_SFH_CMD_STOP_ALL_SENSORS;
		cmd.cmd_v1.interval = 0;
		cmd.cmd_v1.sensor_id = 0;
		break;
	}

	parm.ul = 0;

	writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
	writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);
}

static void amd_sfh_pci_remove(void *privdata)
{
	amd_sfh_client_deinit(privdata);
	amd_sfh_stop_all_sensors(privdata);
}

static int amd_sfh_pci_probe(struct pci_dev *pci_dev,
			     const struct pci_device_id *id)
{
	struct amd_sfh_data *privdata;
	int rc;

	privdata = devm_kzalloc(&pci_dev->dev, sizeof(*privdata), GFP_KERNEL);
	if (!privdata)
		return -ENOMEM;

	privdata->pci_dev = pci_dev;
	pci_set_drvdata(pci_dev, privdata);
	rc = pcim_enable_device(pci_dev);
	if (rc)
		return rc;

	rc = pcim_iomap_regions(pci_dev, BIT(2), DRIVER_NAME);
	if (rc)
		return rc;

	privdata->mmio = pcim_iomap_table(pci_dev)[2];
	pci_set_master(pci_dev);
	rc = pci_set_consistent_dma_mask(pci_dev, DMA_BIT_MASK(64));
	if (rc)
		rc = pci_set_consistent_dma_mask(pci_dev, DMA_BIT_MASK(32));
	if (rc)
		return rc;

	amd_sfh_client_init(privdata);
	return devm_add_action_or_reset(&pci_dev->dev, amd_sfh_pci_remove,
					privdata);
}

static const struct pci_device_id amd_sfh_pci_tbl[] = {
	{ PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_SFH) },
	{ }
};
MODULE_DEVICE_TABLE(pci, amd_sfh_pci_tbl);

static struct pci_driver amd_sfh_pci_driver = {
	.name		= "amd-sfh-pci",
	.id_table	= amd_sfh_pci_tbl,
	.probe		= amd_sfh_pci_probe,
};
module_pci_driver(amd_sfh_pci_driver);

MODULE_DESCRIPTION("AMD(R) Sensor Fusion Hub PCI driver");
MODULE_AUTHOR("Shyam Sundar S K <Shyam-sundar.S-k@amd.com>");
MODULE_AUTHOR("Nehal Bakulchandra Shah <Nehal-bakulchandra.Shah@amd.com>");
MODULE_AUTHOR("Richard Neumann <mail@richard-neumann.de>");
MODULE_LICENSE("Dual BSD/GPL");
