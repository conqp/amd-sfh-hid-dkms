// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 *  AMD Sensor Fusion Hub HID platform driver
 *
 *  Authors: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *           Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "amd-sfh.h"
#include "amd-sfh-hid-ll-drv.h"
#include "amd-sfh-hid-reports.h"

#define AMD_SFH_UPDATE_INTERVAL	200
#define AMD_SFH_HID_VENDOR	0x3fe
#define AMD_SFH_HID_PRODUCT	0x0001
#define AMD_SFH_HID_VERSION	0x0001
#define AMD_SFH_PHY_DEV		"AMD Sensor Fusion Hub (PCIe)"

/**
 * get_sensor_name - Returns the name of a sensor by its index.
 * @sensor_idx:	The sensor's index
 *
 * Returns the name of the respective sensor.
 */
static char *amd_sfh_get_sensor_name(enum sensor_idx sensor_idx)
{
	switch (sensor_idx) {
	case ACCEL_IDX:
		return "accelerometer";
	case GYRO_IDX:
		return "gyroscope";
	case MAG_IDX:
		return "magnetometer";
	case ALS_IDX:
		return "ambient light sensor";
	default:
		return "unknown sensor type";
	}
}

/**
 * amd_sfh_hid_poll - Updates the input report for a HID device.
 * @work:	The delayed work
 *
 * Polls input reports from the respective HID devices and submits
 * them by invoking hid_input_report() from hid-core.
 */
static void amd_sfh_hid_poll(struct work_struct *work)
{
	struct amd_sfh_hid_data *hid_data;
	struct hid_device *hid;
	size_t size;
	u8 *buf;

	hid_data = container_of(work, struct amd_sfh_hid_data, work.work);
	hid = hid_data->hid;
	size = get_descriptor_size(hid_data->sensor_idx, AMD_SFH_INPUT_REPORT);

	buf = kzalloc(size, GFP_KERNEL);
	if (!buf)
		goto reschedule;

	size = get_input_report(hid_data->sensor_idx, 1, buf, size,
				hid_data->cpu_addr);
	if (size < 0) {
		hid_err(hid, "Failed to get input report!\n");
		goto free_buf;
	}

	hid_input_report(hid, HID_INPUT_REPORT, buf, size, 0);

free_buf:
	kfree(buf);
reschedule:
	schedule_delayed_work(&hid_data->work, hid_data->interval);
}

/**
 * amd_sfh_hid_probe - Initializes the respective HID device.
 * @pci_dev:		The underlying PCI device
 * @sensor_idx:		The sensor index
 *
 * Sets up the HID driver data and the corresponding HID device.
 * Returns a pointer to the new HID device or NULL on errors.
 */
static struct hid_device *amd_sfh_hid_probe(struct pci_dev *pci_dev,
					    enum sensor_idx sensor_idx)
{
	int rc;
	char *name;
	struct hid_device *hid;
	struct amd_sfh_hid_data *hid_data;

	hid = hid_allocate_device();
	if (IS_ERR(hid)) {
		pci_err(pci_dev, "Failed to allocate HID device!\n");
		goto err_hid_alloc;
	}

	hid_data = devm_kzalloc(&pci_dev->dev, sizeof(*hid_data), GFP_KERNEL);
	if (!hid_data)
		goto destroy_hid_device;

	hid_data->sensor_idx = sensor_idx;
	hid_data->pci_dev = pci_dev;
	hid_data->hid = hid;
	hid_data->cpu_addr = NULL;
	hid_data->interval = AMD_SFH_UPDATE_INTERVAL;

	INIT_DELAYED_WORK(&hid_data->work, amd_sfh_hid_poll);

	hid->bus = BUS_I2C;
	hid->group = HID_GROUP_SENSOR_HUB;
	hid->vendor = AMD_SFH_HID_VENDOR;
	hid->product = AMD_SFH_HID_PRODUCT;
	hid->version = AMD_SFH_HID_VERSION;
	hid->type = HID_TYPE_OTHER;
	hid->ll_driver = &amd_sfh_hid_ll_driver;
	hid->driver_data = hid_data;

	name = amd_sfh_get_sensor_name(sensor_idx);

	rc = strscpy(hid->name, name, sizeof(hid->name));
	if (rc >= sizeof(hid->name))
		hid_warn(hid, "Could not set HID device name.\n");

	rc = strscpy(hid->phys, AMD_SFH_PHY_DEV, sizeof(hid->phys));
	if (rc >= sizeof(hid->phys))
		hid_warn(hid, "Could not set HID device location.\n");

	rc = hid_add_device(hid);
	if (rc)	{
		hid_err(hid, "Failed to add HID device: %d\n", rc);
		goto free_hid_data;
	}

	return hid;

free_hid_data:
	devm_kfree(&pci_dev->dev, hid_data);
destroy_hid_device:
	hid_destroy_device(hid);
err_hid_alloc:
	return NULL;
}

/**
 * amd_sfh_client_init - Initializes the HID devices.
 * @privdata:	The driver data
 *
 * Matches the sensors's masks against the sensor mask retrieved
 * from amd_sfh_get_sensor_mask().
 * In case of a match, it instantiates a corresponding HID device
 * to process the respective sensor's data.
 *
 * Always returns 0.
 */
int amd_sfh_client_init(struct amd_sfh_data *privdata)
{
	struct pci_dev *pci_dev;
	uint sensor_mask;
	int i = -1;

	pci_dev = privdata->pci_dev;
	sensor_mask = amd_sfh_get_sensor_mask(pci_dev);

	if (sensor_mask & ACCEL_MASK)
		privdata->sensors[i++] = amd_sfh_hid_probe(pci_dev, ACCEL_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & GYRO_MASK)
		privdata->sensors[i++] = amd_sfh_hid_probe(pci_dev, GYRO_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & MAGNO_MASK)
		privdata->sensors[i++] = amd_sfh_hid_probe(pci_dev, MAG_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & ALS_MASK)
		privdata->sensors[i++] = amd_sfh_hid_probe(pci_dev, ALS_IDX);
	else
		privdata->sensors[i++] = NULL;

	return 0;
}

/**
 * amd_sfh_client_deinit - Removes all active HID devices.
 * @privdata:	The driver data
 *
 * Destroys all initialized HID devices.
 * Always returns 0.
 */
int amd_sfh_client_deinit(struct amd_sfh_data *privdata)
{
	int i;

	pci_err(pci_dev, "Deiniting all sensors...\n");
	for (i = 0; i < AMD_SFH_MAX_HID_DEVICES; i++) {
		pci_err(pci_dev, "Deiniting sensor %i.\n", i);

		if (privdata->sensors[i])
			hid_destroy_device(privdata->sensors[i]);

		privdata->sensors[i] = NULL;
	}

	return 0;
}
