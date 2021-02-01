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
#include "amd-sfh-pci.h"
#include "sensors/amd-sfh-accel.h"
#include "sensors/amd-sfh-als.h"
#include "sensors/amd-sfh-gyro.h"
#include "sensors/amd-sfh-mag.h"

#define AMD_SFH_HID_DMA_SIZE	(sizeof(int) * 8)

/**
 * poll - Updates the input report for a HID device.
 * @work:	The delayed work
 *
 * Polls input reports from the respective HID devices and submits
 * them by invoking hid_input_report() from hid-core.
 */
static void poll(struct work_struct *work)
{
	struct amd_sfh_hid_data *hid_data;
	struct hid_report *report;

	hid_data = container_of(work, struct amd_sfh_hid_data, work.work);
	hid_err(hid_data->hid, "poll");

	report = hid_register_report(hid_data->hid, HID_INPUT_REPORT, 1, 0);
	if (!report)
		goto reschedule;

	hid_hw_request(hid_data->hid, report, HID_REQ_GET_REPORT);

reschedule:
	schedule_delayed_work(&hid_data->work, AMD_SFH_UPDATE_INTERVAL);
}

/**
 * parse - Callback to parse HID descriptor.
 * @hid:	The HID device
 *
 * This function gets called during call to hid_add_device
 *
 * Return: 0 on success and non zero on error.
 */
static int parse(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_err(hid, "parse");
	switch (hid_data->sensor_idx) {
	case ACCEL_IDX:
		return amd_sfh_parse_accel(hid);
	case ALS_IDX:
		return amd_sfh_parse_als(hid);
	case GYRO_IDX:
		return amd_sfh_parse_gyro(hid);
	case MAG_IDX:
		return amd_sfh_parse_mag(hid);
	default:
		return -EINVAL;
	}
}

/**
 * start - Starts the HID device.
 * @hid:	The HID device
 *
 * Allocates DMA memory on the PCI device.
 * Returns 0 on success and non-zero on error.
 */
static int start(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_data->cpu_addr = dma_alloc_coherent(&hid_data->pci_dev->dev,
						AMD_SFH_HID_DMA_SIZE,
						&hid_data->dma_handle,
						GFP_KERNEL);
	if (!hid_data->cpu_addr)
		return -EIO;

	INIT_DELAYED_WORK(&hid_data->work, poll);
	return 0;
}

/**
 * stop - Stops the HID device.
 * @hid:	The HID device
 *
 * Frees the DMA memory on the PCI device.
 */
static void stop(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_err(hid, "stop");
	dma_free_coherent(&hid_data->pci_dev->dev, AMD_SFH_HID_DMA_SIZE,
			  hid_data->cpu_addr, hid_data->dma_handle);
	hid_data->cpu_addr = NULL;
}

/**
 * open - Opens the HID device.
 * @hid:	The HID device
 *
 * Starts the corresponding sensor via the PCI driver
 * and schedules report polling.
 * Always returns 0.
 */
static int open(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_err(hid, "open");
	amd_sfh_start_sensor(hid_data->pci_dev, hid_data->sensor_idx,
			     hid_data->dma_handle);
	schedule_delayed_work(&hid_data->work, AMD_SFH_UPDATE_INTERVAL);
	return 0;
}

/**
 * close - Closes the HID device.
 * @hid:	The HID device
 *
 * Stops report polling and the corresponding sensor via the PCI driver.
 */
static void close(struct hid_device *hid)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_err(hid, "close");
	cancel_delayed_work_sync(&hid_data->work);
	amd_sfh_stop_sensor(hid_data->pci_dev, hid_data->sensor_idx);
}

/**
 * raw_request - Handles HID requests.
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
static int raw_request(struct hid_device *hid, unsigned char reportnum, u8 *buf,
		       size_t len, unsigned char rtype, int reqtype)
{
	struct amd_sfh_hid_data *hid_data = hid->driver_data;

	hid_err(hid, "raw_request: num: %u, len: %lu, rtype: %u, reqtype: %d",
		reportnum, len, rtype, reqtype);
	switch (rtype) {
	case HID_FEATURE_REPORT:
		switch (hid_data->sensor_idx) {
		case ACCEL_IDX:
			return amd_sfh_get_accel_feature_report\
				(reportnum, buf, len);
		case ALS_IDX:
			return amd_sfh_get_als_feature_report\
				(reportnum, buf, len);
		case GYRO_IDX:
			return amd_sfh_get_gyro_feature_report\
				(reportnum, buf, len);
		case MAG_IDX:
			return amd_sfh_get_mag_feature_report\
				(reportnum, buf, len);
		default:
			return -EINVAL;
		}
	case HID_INPUT_REPORT:
		switch (hid_data->sensor_idx) {
		case ACCEL_IDX:
			return amd_sfh_get_accel_input_report\
				(reportnum, buf, len, hid_data->cpu_addr);
		case ALS_IDX:
			return amd_sfh_get_als_input_report\
				(reportnum, buf, len, hid_data->cpu_addr);
		case GYRO_IDX:
			return amd_sfh_get_gyro_input_report\
				(reportnum, buf, len, hid_data->cpu_addr);
		case MAG_IDX:
			return amd_sfh_get_mag_input_report\
				(reportnum, buf, len, hid_data->cpu_addr);
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
	.parse	=	parse,
	.start	=	start,
	.stop	=	stop,
	.open	=	open,
	.close	=	close,
	.raw_request  =	raw_request,
};
