// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub PCIe driver
 *
 * Authors: Shyam Sundar S K <Shyam-sundar.S-k@amd.com>
 *          Nehal Bakulchandra Shah <Nehal-bakulchandra.Shah@amd.com>
 *          Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/io-64-nonatomic-lo-hi.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/types.h>

#include "amd-sfh.h"
#include "amd-sfh-client.h"
#include "amd-sfh-pci.h"
#include "amd-sfh-quirks.h"

#define DRIVER_NAME		"amd-sfh"
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
	uint sensor_mask;
	struct amd_sfh_data *privdata;
	const struct amd_sfh_quirks *quirks;

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
	int cmd_id, sid;

	privdata = pci_get_drvdata(pci_dev);

	cmd.ul = 0;
	cmd.s.cmd_id = AMD_SFH_CMD_ENABLE_SENSOR;
	cmd.s.interval = AMD_SFH_UPDATE_INTERVAL;
	cmd.s.sensor_id = sensor_idx;

	parm.ul = 0;
	parm.s.buffer_layout = 1;
	parm.s.buffer_length = 16;

	writeq(dma_handle, privdata->mmio + AMD_C2P_MSG2);
	writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
	writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);

	msleep(1000);

	for (cmd_id = AMD_SFH_CMD_DUMP_SENSOR_INFO; cmd_id < AMD_SFH_CMD_INVALID; cmd_id++) {
		for (sid = ACCEL_IDX; sid <= ALS_IDX; sid++) {
			pci_err(pci_dev, "Enabling interrupts for: %d", sid);
			writel(1, privdata->mmio + AMD_P2C_MSG_INTEN);
			msleep(1000);
			pci_err(pci_dev, "Sending command: %d", cmd_id);
			cmd.ul = 0;
			cmd.s.cmd_id = cmd_id;
			cmd.s.sensor_id = sensor_idx;
			parm.ul = 0;
			writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
			writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);
			msleep(1000);
		}
	}
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
	cmd.s.cmd_id = AMD_SFH_CMD_DISABLE_SENSOR;
	cmd.s.interval = 0;
	cmd.s.sensor_id = sensor_idx;

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
	cmd.s.cmd_id = AMD_SFH_CMD_STOP_ALL_SENSORS;
	cmd.s.interval = 0;
	cmd.s.sensor_id = 0;

	parm.ul = 0;

	writel(parm.ul, privdata->mmio + AMD_C2P_MSG1);
	writel(cmd.ul, privdata->mmio + AMD_C2P_MSG0);
}

static void amd_sfh_pci_remove(void *privdata)
{
	amd_sfh_client_deinit(privdata);
	amd_sfh_stop_all_sensors(privdata);
}

/**
 * amd_sfh_reset_interrupts - Resets the interrupt registers.
 * @privdata:	The driver data
 */
static void amd_sfh_reset_interrupts(struct amd_sfh_data *privdata)
{
	int inten;

	inten = readl(privdata->mmio + AMD_P2C_MSG_INTEN);
	if (inten)
		writel(0, privdata->mmio + AMD_P2C_MSG_INTEN);
}

/**
 * amd_sfh_irq_isr - Handles interrupts.
 * @irq:	The IRQ received
 * @dev:	The driver data
 *
 * XXX: Disables IRQ handling to prevent IRQ flooding.
 * Reads response information from relevant P2C registers.
 * Releases lock to allow next command to be executed.
 */
static irqreturn_t amd_sfh_irq_isr(int irq, void *dev)
{
	int event, debuginfo1, debuginfo2, activecontrolstatus;
	struct amd_sfh_data *privdata = dev;

	pci_err(privdata->pci_dev, "Disabling interrupts.");
	amd_sfh_reset_interrupts(privdata);

	/* Read response registers */
	event = readl(privdata->mmio + AMD_P2C_MSG0);
	debuginfo1 = readl(privdata->mmio + AMD_P2C_MSG1);
	debuginfo2 = readl(privdata->mmio + AMD_P2C_MSG2);
	activecontrolstatus = readl(privdata->mmio + AMD_P2C_MSG3);

	pci_err(privdata->pci_dev,
		"Received interrupt %d: event: %d, debuginfo1: %d, debuginfo2: %d, acs: %d.\n",
		irq, event, debuginfo1, debuginfo2, activecontrolstatus);

	return IRQ_HANDLED;
}

static int amd_sfh_pci_probe(struct pci_dev *pci_dev,
			     const struct pci_device_id *id)
{
	int rc;
	struct amd_sfh_data *privdata;

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

	rc = pci_set_dma_mask(pci_dev, DMA_BIT_MASK(64));
	if (rc)
		rc = pci_set_dma_mask(pci_dev, DMA_BIT_MASK(32));
	if (rc)
		return rc;

	rc = devm_request_irq(&pci_dev->dev, pci_dev->irq, amd_sfh_irq_isr,
			      IRQF_SHARED, pci_name(pci_dev), privdata);
	if (rc)
		return rc;

	rc = devm_add_action_or_reset(&pci_dev->dev, amd_sfh_pci_remove,
				      privdata);
	if (rc)
		return rc;

	amd_sfh_client_init(privdata);
	return rc;
}

static const struct pci_device_id amd_sfh_pci_tbl[] = {
	{ PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_SFH) },
	{ }
};
MODULE_DEVICE_TABLE(pci, amd_sfh_pci_tbl);

static struct pci_driver amd_sfh_pci_driver = {
	.name		= DRIVER_NAME,
	.id_table	= amd_sfh_pci_tbl,
	.probe		= amd_sfh_pci_probe,
};
module_pci_driver(amd_sfh_pci_driver);

MODULE_DESCRIPTION("AMD(R) Sensor Fusion Hub PCI driver");
MODULE_AUTHOR("Shyam Sundar S K <Shyam-sundar.S-k@amd.com>");
MODULE_AUTHOR("Nehal Bakulchandra Shah <Nehal-bakulchandra.Shah@amd.com>");
MODULE_AUTHOR("Richard Neumann <mail@richard-neumann.de>");
MODULE_LICENSE("Dual BSD/GPL");
