// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 *  AMD Sensor Fusion Hub HID platform driver
 *
 *  Authors: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *           Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/acpi.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "amd-sfh-hid-ll-drv.h"
#include "amd-sfh-hid-reports.h"
#include "amd-sfh-plat.h"
#include "amd-sfh-quirks.h"

#define AMD_SFH_UPDATE_INTERVAL	200
#define AMD_SFH_HID_VENDOR	0x3fe
#define AMD_SFH_HID_PRODUCT	0x0001
#define AMD_SFH_HID_VERSION	0x0001
#define AMD_SFH_PHY_DEV		"AMD Sensor Fusion Hub (PCIe)"

/* Module parameters */
static int sensor_mask_override = -1;
module_param_named(sensor_mask, sensor_mask_override, int, 0644);
MODULE_PARM_DESC(sensor_mask, "override the detected sensors mask");

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
 * amd_sfh_plat_get_sensor_mask - Returns the sensors mask.
 * @pci_dev:	The SFH PCI device
 *
 * Gets the sensor mask from the PCI device.
 * Optionally overrides that value with the value provided by the
 * kernel parameter `sensor_mask_override`.
 * If no sensors were discovered, it returns the sensors
 * as specified in the quirks.
 */
static int amd_sfh_plat_get_sensor_mask(struct pci_dev *pci_dev)
{
	int sensor_mask = amd_sfh_get_sensor_mask(pci_dev);

	if (sensor_mask_override >= 0)
		return sensor_mask_override;

	if (!sensor_mask)
		return amd_sfh_quirks_get_sensor_mask();

	return sensor_mask;
}

/**
 * init_hid_devices - Initializes the HID devices.
 * @privdata:	The platform device data
 *
 * Matches the sensors's masks against the sensor mask retrieved
 * from amd_sfh_plat_get_sensor_mask().
 * In case of a match, it instantiates a corresponding HID device
 * to process the respective sensor's data.
 */
static void amd_sfh_init_hid_devices(struct amd_sfh_plat_dev *privdata)
{
	struct pci_dev *pci_dev;
	int sensor_mask;

	pci_dev = privdata->pci_dev;
	sensor_mask = amd_sfh_plat_get_sensor_mask(pci_dev);

	if (sensor_mask & ACCEL_MASK)
		privdata->accel = amd_sfh_hid_probe(pci_dev, ACCEL_IDX);
	else
		privdata->accel = NULL;

	if (sensor_mask & GYRO_MASK)
		privdata->gyro = amd_sfh_hid_probe(pci_dev, GYRO_IDX);
	else
		privdata->gyro = NULL;

	if (sensor_mask & MAGNO_MASK)
		privdata->magno = amd_sfh_hid_probe(pci_dev, MAG_IDX);
	else
		privdata->magno = NULL;

	if (sensor_mask & ALS_MASK)
		privdata->als = amd_sfh_hid_probe(pci_dev, ALS_IDX);
	else
		privdata->als = NULL;
}

/**
 * remove_hid_devices - Removes all active HID devices.
 * @privdata:	The platform device data
 *
 * Destroys all initialized HID devices.
 */
static void remove_hid_devices(struct amd_sfh_plat_dev *privdata)
{
	if (privdata->accel)
		hid_destroy_device(privdata->accel);

	privdata->accel = NULL;

	if (privdata->gyro)
		hid_destroy_device(privdata->gyro);

	privdata->gyro = NULL;

	if (privdata->magno)
		hid_destroy_device(privdata->magno);

	privdata->magno = NULL;

	if (privdata->als)
		hid_destroy_device(privdata->als);

	privdata->als = NULL;
}

/**
 * amd_sfh_platform_probe - Probes the AMD SFH platform driver
 * @pdev:	The platform device
 *
 * Initializes the client data and invokes initialization of HID devices.
 * Returns 0 on success and nonzero on errors.
 */
static int amd_sfh_platform_probe(struct platform_device *pdev)
{
	struct amd_sfh_plat_dev *privdata;
	struct pci_dev *pci_dev;

	privdata = devm_kzalloc(&pdev->dev, sizeof(*privdata), GFP_KERNEL);
	if (!privdata)
		return -ENOMEM;

	pci_dev = pci_get_device(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_SFH, NULL);
	if (!pci_dev) {
		dev_err(&pdev->dev, "No matching PCI device found!\n");
		return -ENODEV;
	}

	privdata->pci_dev = pci_dev;
	platform_set_drvdata(pdev, privdata);
	amd_sfh_init_hid_devices(privdata);
	return 0;
}

/**
 * amd_sfh_platform_remove - Removes AMD SFH platform driver
 * @pdev:	The platform device
 *
 * Removes the HID devices and unloads the driver.
 * Returns 0 on success and nonzero on errors.
 */
static int amd_sfh_platform_remove(struct platform_device *pdev)
{
	struct amd_sfh_plat_dev *privdata;

	privdata = platform_get_drvdata(pdev);
	if (!privdata)
		return -EINVAL;

	remove_hid_devices(privdata);
	return 0;
}

static const struct acpi_device_id amd_sfh_acpi_match[] = {
	{ "AMDI0080" },
	{ },
};

MODULE_DEVICE_TABLE(acpi, amd_sfh_acpi_match);

static struct platform_driver amd_sfh_platform_driver = {
	.probe = amd_sfh_platform_probe,
	.remove = amd_sfh_platform_remove,
	.driver = {
		.name = "amd-sfh-hid",
		.acpi_match_table = amd_sfh_acpi_match,
	},
};

module_platform_driver(amd_sfh_platform_driver);

MODULE_DESCRIPTION("AMD(R) Sensor Fusion Hub HID platform driver");
MODULE_AUTHOR("Nehal Shah <nehal-bakulchandra.shah@amd.com>");
MODULE_AUTHOR("Richard Neumann <mail@richard-neumann.de>");
MODULE_LICENSE("Dual BSD/GPL");
