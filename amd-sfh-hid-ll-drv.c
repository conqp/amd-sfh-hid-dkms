// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub HID low-level driver
 *
 * Authors: Sandeep Singh <sandeep.singh@amd.com>
 *          Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 *          Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/dma-mapping.h>
#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

#include "amd-sfh.h"
#include "amd-sfh-hid-ll-drv.h"
#include "amd-sfh-hid-reports.h"
#include "amd-sfh-pci.h"
#include "amd-sfh-report-descriptors.h"

#define AMD_SFH_HID_DMA_SIZE	(sizeof(int) * 8)

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

	hid_data = container_of(work, struct amd_sfh_hid_data, work.work);
	hid_hw_raw_request(hid_data->hid, 1, hid_data->report_buf,
			   hid_data->report_size, HID_INPUT_REPORT, 0);
	schedule_delayed_work(&hid_data->work, AMD_SFH_UPDATE_INTERVAL);
}

/**
 * amd_sfh_hid_ll_parse - Callback to parse HID descriptor.
 * @hid:	The HID device
 *
 * This function gets called during call to hid_add_device
 *
 * Return: 0 on success and non zero on error.
 */
static int amd_sfh_hid_ll_parse(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data;
	size_t size;
	u8 *buf;
	int rc;

	hid_data = hid->driver_data;

	switch (hid_data->sensor_idx) {
	case ACCEL_IDX:
		size = accel3_report_descriptor_size;
		buf = kstrndup(accel3_report_descriptor, size, GFP_KERNEL);
		break;
	case GYRO_IDX:
		size = gyro3_report_descriptor_size;
		buf = kstrndup(gyro3_report_descriptor, size, GFP_KERNEL);
		break;
	case MAG_IDX:
		size = magno_report_descriptor_size;
		buf = kstrndup(magno_report_descriptor, size, GFP_KERNEL);
		break;
	case ALS_IDX:
		size = als_report_descriptor_size;
		buf = kstrndup(als_report_descriptor, size, GFP_KERNEL);
		break;
	default:
		return -EINVAL;
	}

	rc = hid_parse_report(hid, buf, size);
	kfree(buf);
	return rc;
}

/**
 * amd_sfh_hid_ll_start - Starts the HID device.
 * @hid:	The HID device
 *
 * Allocates DMA memory on the PCI device.
 * Returns 0 on success and non-zero on error.
 */
static int amd_sfh_hid_ll_start(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_data->cpu_addr = dma_alloc_coherent(&hid_data->pci_dev->dev,
						AMD_SFH_HID_DMA_SIZE,
						&hid_data->dma_handle,
						GFP_KERNEL);
	if (!hid_data->cpu_addr)
		return -EIO;

	INIT_DELAYED_WORK(&hid_data->work, amd_sfh_hid_poll);
	return 0;
}

/**
 * amd_sfh_hid_ll_stop - Stops the HID device.
 * @hid:	The HID device
 *
 * Frees the DMA memory on the PCI device.
 */
static void amd_sfh_hid_ll_stop(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	dma_free_coherent(&hid_data->pci_dev->dev, AMD_SFH_HID_DMA_SIZE,
			  hid_data->cpu_addr, hid_data->dma_handle);
	hid_data->cpu_addr = NULL;
}

/**
 * amd_sfh_hid_ll_open - Opens the HID device.
 * @hid:	The HID device
 *
 * Starts the corresponding sensor via the PCI driver
 * and schedules report polling.
 * Always returns 0.
 */
static int amd_sfh_hid_ll_open(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	amd_sfh_start_sensor(hid_data->pci_dev, hid_data->sensor_idx,
			     hid_data->dma_handle);
	schedule_delayed_work(&hid_data->work, AMD_SFH_UPDATE_INTERVAL);
	return 0;
}

/**
 * amd_sfh_hid_ll_close - Closes the HID device.
 * @hid:	The HID device
 *
 * Stops report polling and the corresponding sensor via the PCI driver.
 */
static void amd_sfh_hid_ll_close(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	cancel_delayed_work_sync(&hid_data->work);
	amd_sfh_stop_sensor(hid_data->pci_dev, hid_data->sensor_idx);
}

/**
 * amd_sfh_hid_ll_raw_request - Handles HID requests.
 * @hid:	The HID device
 * @reportnum:	The HID report ID
 * @buf:	The write buffer for HID data
 * @len:	The size of the write buffer
 * @rtype:	The report type
 * @reqtype:	The request type
 *
 * Delegates to the reporting functions
 * defined in amd-sfh-hid-descriptor.h.
 */
static int amd_sfh_hid_ll_raw_request(struct hid_device *hid,
				      unsigned char reportnum, u8 *buf,
				      size_t len, unsigned char rtype,
				      int reqtype)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	switch (rtype) {
	case HID_FEATURE_REPORT:
		if (!buf)
			return -ENOBUFS;

		switch (hid_data->sensor_idx) {
		case ACCEL_IDX:
			return get_accel_feature_report(reportnum, buf, len);
		case GYRO_IDX:
			return get_gyro_feature_report(reportnum, buf, len);
		case MAG_IDX:
			return get_mag_feature_report(reportnum, buf, len);
		case ALS_IDX:
			return get_als_feature_report(reportnum, buf, len);
		default:
			return -EINVAL;
		}
	case HID_INPUT_REPORT:
		if (!buf)
			return -ENOBUFS;

		if (!hid_data->cpu_addr)
			return -EIO;

		switch (hid_data->sensor_idx) {
		case ACCEL_IDX:
			return get_accel_input_report(reportnum, buf, len,
						      hid_data->cpu_addr);
		case GYRO_IDX:
			return get_gyro_input_report(reportnum, buf, len,
						     hid_data->cpu_addr);
		case MAG_IDX:
			return get_mag_input_report(reportnum, buf, len,
						    hid_data->cpu_addr);
		case ALS_IDX:
			return get_als_input_report(reportnum, buf, len,
						    hid_data->cpu_addr);
		default:
			return -EINVAL;
		}
	default:
		return -EINVAL;
	}
}

/**
 * The HID low-level driver for SFH HID devices.
 */
struct hid_ll_driver amd_sfh_hid_ll_driver = {
	.parse	=	amd_sfh_hid_ll_parse,
	.start	=	amd_sfh_hid_ll_start,
	.stop	=	amd_sfh_hid_ll_stop,
	.open	=	amd_sfh_hid_ll_open,
	.close	=	amd_sfh_hid_ll_close,
	.raw_request  =	amd_sfh_hid_ll_raw_request,
};
