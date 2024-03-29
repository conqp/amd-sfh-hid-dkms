// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 *  AMD Sensor Fusion Hub HID client
 *
 *  Authors:	Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "amd-sfh.h"
#include "amd-sfh-client.h"
#include "amd-sfh-hid-ll-drv.h"
#include "amd-sfh-pci.h"

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
static char *get_sensor_name(enum sensor_idx sensor_idx)
{
	switch (sensor_idx) {
	case ACCEL_IDX:
		return "accelerometer";
	case GYRO_IDX:
		return "gyroscope";
	case MAG_IDX:
		return "magnetometer";
	case LID_IDX:
		return "lid switch";
	case ALS_IDX:
		return "ambient light sensor";
	default:
		return "unknown sensor type";
	}
}

/**
 * get_hid_data - Allocate and initialize HID device driver data.
 * @hid:		HID device
 * @privdata:		SFH driver data
 * @sensor_idx:		Sensor index
 *
 * Returns a pointer to the HID driver data on success or an ERR_PTR on error.
 */
static struct amd_sfh_hid_data *get_hid_data(struct hid_device *hid,
					     struct amd_sfh_data *privdata,
					     enum sensor_idx sensor_idx)
{
	struct amd_sfh_hid_data *hid_data;

	hid_data = devm_kzalloc(&privdata->pci_dev->dev, sizeof(*hid_data),
				GFP_KERNEL);
	if (!hid_data)
		return ERR_PTR(-ENOMEM);

	hid_data->hid = hid;
	hid_data->pci_dev = privdata->pci_dev;
	hid_data->version = privdata->version;
	hid_data->sensor_idx = sensor_idx;
	hid_data->cpu_addr = NULL;

	return hid_data;
}

/**
 * get_hid_device - Creates a HID device for a sensor on th SFH.
 * @privdata:		SFH driver data
 * @sensor_idx:		Sensor index
 *
 * Sets up the HID device and the corresponding HID driver data.
 * Returns a pointer to the new HID device or NULL on errors.
 */
static struct hid_device *get_hid_device(struct amd_sfh_data *privdata,
					 enum sensor_idx sensor_idx)
{
	struct hid_device *hid;
	int rc;

	hid = hid_allocate_device();
	if (IS_ERR(hid)) {
		pci_err(privdata->pci_dev, "HID device allocation returned: %ld",
			PTR_ERR(hid));
		goto err_hid_alloc;
	}

	hid->bus = BUS_I2C;
	hid->group = HID_GROUP_SENSOR_HUB;
	hid->vendor = AMD_SFH_HID_VENDOR;
	hid->product = AMD_SFH_HID_PRODUCT;
	hid->version = AMD_SFH_HID_VERSION;
	hid->type = HID_TYPE_OTHER;
	hid->ll_driver = &amd_sfh_hid_ll_driver;

	rc = strscpy(hid->phys, AMD_SFH_PHY_DEV, sizeof(hid->phys));
	if (rc >= sizeof(hid->phys))
		hid_warn(hid, "Could not set HID device location.\n");

	rc = strscpy(hid->name, get_sensor_name(sensor_idx), sizeof(hid->name));
	if (rc >= sizeof(hid->name))
		hid_warn(hid, "Could not set HID device name.\n");

	hid->driver_data = get_hid_data(hid, privdata, sensor_idx);
	if (IS_ERR(hid->driver_data)) {
		hid_err(hid, "HID data allocation returned: %ld",
			PTR_ERR(hid->driver_data));
		goto destroy_hid_device;
	}

	rc = hid_add_device(hid);
	if (rc)	{
		hid_err(hid, "Failed to add HID device: %d\n", rc);
		goto free_hid_data;
	}

	return hid;

free_hid_data:
	devm_kfree(&privdata->pci_dev->dev, hid->driver_data);
destroy_hid_device:
	hid_destroy_device(hid);
err_hid_alloc:
	return NULL;
}

/**
 * amd_sfh_client_init - Initializes the HID devices.
 * @privdata:		SFH driver data
 *
 * Matches the sensor bitmasks against the sensor bitmask retrieved
 * from amd_sfh_get_sensor_mask().
 * In case of a match, it instantiates a corresponding HID device
 * to process the respective sensor's data.
 */
void amd_sfh_client_init(struct amd_sfh_data *privdata)
{
	struct pci_dev *pci_dev = privdata->pci_dev;
	uint sensor_mask = amd_sfh_get_sensor_mask(pci_dev);
	int i = 0;

	if (sensor_mask & ACCEL_MASK)
		privdata->sensors[i++] = get_hid_device(privdata, ACCEL_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & GYRO_MASK)
		privdata->sensors[i++] = get_hid_device(privdata, GYRO_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & MAG_MASK)
		privdata->sensors[i++] = get_hid_device(privdata, MAG_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & LID_MASK)
		privdata->sensors[i++] = get_hid_device(privdata, LID_IDX);
	else
		privdata->sensors[i++] = NULL;

	if (sensor_mask & ALS_MASK)
		privdata->sensors[i++] = get_hid_device(privdata, ALS_IDX);
	else
		privdata->sensors[i++] = NULL;
}

/**
 * amd_sfh_client_deinit - Removes all active HID devices.
 * @privdata:	Driver data
 *
 * Destroys all initialized HID devices.
 */
void amd_sfh_client_deinit(struct amd_sfh_data *privdata)
{
	int i;

	for (i = 0; i < AMD_SFH_MAX_SENSORS; i++) {
		if (privdata->sensors[i])
			hid_destroy_device(privdata->sensors[i]);

		privdata->sensors[i] = NULL;
	}
}
