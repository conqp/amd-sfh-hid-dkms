// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 *  AMD SFH HID report and descriptor generation
 *
 *  Author: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *          Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "amd-sfh-hid-descriptors.h"
#include "amd-sfh-hid-reports.h"
#include "amd-sfh-pci.h"

#define FIRMWARE_MULTIPLIER						1000
#define HID_USAGE_SENSOR_PROP_REPORTING_STATE_ALL_EVENTS_ENUM		0x41
#define HID_USAGE_SENSOR_PROP_POWER_STATE_D0_FULL_POWER_ENUM		0x51
#define HID_DEFAULT_REPORT_INTERVAL					0x50
#define HID_DEFAULT_MIN_VALUE						0X7F
#define HID_DEFAULT_MAX_VALUE						0x80
#define HID_DEFAULT_SENSITIVITY						0x7F
#define HID_USAGE_SENSOR_PROPERTY_CONNECTION_TYPE_PC_INTEGRATED_ENUM	0x01
/* state enums */
#define HID_USAGE_SENSOR_STATE_READY_ENUM				0x02
#define HID_USAGE_SENSOR_STATE_INITIALIZING_ENUM			0x05
#define HID_USAGE_SENSOR_EVENT_DATA_UPDATED_ENUM			0x04

/**
 * get_report_descriptor - Writes a report descriptor.
 * @sensor_idx:		The sensor index
 * @buf:		The report descriptor buffer
 *
 * Returns zero on success or nonzero on errors.
 */
int get_report_descriptor(enum sensor_idx sensor_idx, u8 *buf)
{
	size_t size;

	if (!buf)
		return -ENOBUFS;

	switch (sensor_idx) {
	case ACCEL_IDX:
		size = sizeof(accel3_report_descriptor);
		memcpy(buf, accel3_report_descriptor, size);
		break;
	case GYRO_IDX:
		size = sizeof(gyro3_report_descriptor);
		memcpy(buf, gyro3_report_descriptor, size);
		break;
	case MAG_IDX:
		size = sizeof(magno_report_descriptor);
		memcpy(buf, magno_report_descriptor, size);
		break;
	case ALS_IDX:
		size = sizeof(als_report_descriptor);
		memcpy(buf, als_report_descriptor, size);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(get_report_descriptor);

/**
 * get_descriptor_size - Returns the requested descriptor size.
 * @sensor_idx:		The sensor index
 * @descriptor_name:	The requested descriptor
 *
 * Returns the respective descriptor's size or <0 on errors.
 */
int get_descriptor_size(enum sensor_idx sensor_idx, enum desc_type desc_type)
{
	switch (sensor_idx) {
	case ACCEL_IDX:
		switch (desc_type) {
		case AMD_SFH_DESCRIPTOR:
			return sizeof(accel3_report_descriptor);
		case AMD_SFH_INPUT_REPORT:
			return sizeof(struct accel3_input_report);
		case AMD_SFH_FEATURE_REPORT:
			return sizeof(struct accel3_feature_report);
		}
		break;
	case GYRO_IDX:
		switch (desc_type) {
		case AMD_SFH_DESCRIPTOR:
			return sizeof(gyro3_report_descriptor);
		case AMD_SFH_INPUT_REPORT:
			return sizeof(struct gyro_input_report);
		case AMD_SFH_FEATURE_REPORT:
			return sizeof(struct gyro_feature_report);
		}
		break;
	case MAG_IDX:
		switch (desc_type) {
		case AMD_SFH_DESCRIPTOR:
			return sizeof(magno_report_descriptor);
		case AMD_SFH_INPUT_REPORT:
			return sizeof(struct magno_input_report);
		case AMD_SFH_FEATURE_REPORT:
			return sizeof(struct magno_feature_report);
		}
		break;
	case ALS_IDX:
		switch (desc_type) {
		case AMD_SFH_DESCRIPTOR:
			return sizeof(als_report_descriptor);
		case AMD_SFH_INPUT_REPORT:
			return sizeof(struct als_input_report);
		case AMD_SFH_FEATURE_REPORT:
			return sizeof(struct als_feature_report);
		}
		break;
	}

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(get_descriptor_size);

static void get_common_features(struct common_features *common, int report_id)
{
	common->report_id = report_id;
	common->connection_type =
		HID_USAGE_SENSOR_PROPERTY_CONNECTION_TYPE_PC_INTEGRATED_ENUM;
	common->report_state =
		HID_USAGE_SENSOR_PROP_REPORTING_STATE_ALL_EVENTS_ENUM;
	common->power_state =
		HID_USAGE_SENSOR_PROP_POWER_STATE_D0_FULL_POWER_ENUM;
	common->sensor_state = HID_USAGE_SENSOR_STATE_INITIALIZING_ENUM;
	common->report_interval =  HID_DEFAULT_REPORT_INTERVAL;
}

static int get_accel_feature_report(int report_id, u8 *buf, size_t len)
{
	size_t size;
	struct accel3_feature_report accel_features;

	get_common_features(&accel_features.common, report_id);
	accel_features.change_sesnitivity = HID_DEFAULT_SENSITIVITY;
	accel_features.sensitivity_min = HID_DEFAULT_MIN_VALUE;
	accel_features.sensitivity_max = HID_DEFAULT_MAX_VALUE;
	size = sizeof(accel_features);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &accel_features, size);
	return size;
}

static int get_gyro_feature_report(int report_id, u8 *buf, size_t len)
{
	size_t size;
	struct gyro_feature_report gyro_features;

	get_common_features(&gyro_features.common, report_id);
	gyro_features.change_sesnitivity = HID_DEFAULT_SENSITIVITY;
	gyro_features.sensitivity_min = HID_DEFAULT_MIN_VALUE;
	gyro_features.sensitivity_max = HID_DEFAULT_MAX_VALUE;
	size = sizeof(gyro_features);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &gyro_features, size);
	return size;
}

static int get_mag_feature_report(int report_id, u8 *buf, size_t len)
{
	size_t size;
	struct magno_feature_report magno_features;

	get_common_features(&magno_features.common, report_id);
	magno_features.heading_min = HID_DEFAULT_MIN_VALUE;
	magno_features.heading_max = HID_DEFAULT_MAX_VALUE;
	magno_features.flux_change_sensitivity = HID_DEFAULT_MIN_VALUE;
	magno_features.flux_min = HID_DEFAULT_MIN_VALUE;
	magno_features.flux_max = HID_DEFAULT_MAX_VALUE;
	size = sizeof(magno_features);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &magno_features, size);
	return size;
}

static int get_als_feature_report(int report_id, u8 *buf, size_t len)
{
	size_t size;
	struct als_feature_report als_features;

	get_common_features(&als_features.common, report_id);
	als_features.change_sesnitivity = HID_DEFAULT_SENSITIVITY;
	als_features.sensitivity_min = HID_DEFAULT_MIN_VALUE;
	als_features.sensitivity_max = HID_DEFAULT_MAX_VALUE;
	size = sizeof(als_features);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &als_features, size);
	return size;
}

/**
 * get_feature_report - Writes a feature report and returns its size.
 * @sensor_idx:		The sensor index
 * @report_id:		The report id
 * @buf:		The feature report buffer
 *
 * Returns the size on success or < 0 on errors.
 */
int get_feature_report(enum sensor_idx sensor_idx, int report_id, u8 *buf,
		       size_t len)
{
	if (!buf)
		return -ENOBUFS;

	switch (sensor_idx) {
	case ACCEL_IDX:
		return get_accel_feature_report(report_id, buf, len);
	case GYRO_IDX:
		return get_gyro_feature_report(report_id, buf, len);
	case MAG_IDX:
		return get_mag_feature_report(report_id, buf, len);
	case ALS_IDX:
		return get_als_feature_report(report_id, buf, len);
	default:
		return -EINVAL;
	}
}
EXPORT_SYMBOL_GPL(get_feature_report);

static void get_common_inputs(struct common_inputs *common, int report_id)
{
	common->report_id = report_id;
	common->sensor_state = HID_USAGE_SENSOR_STATE_READY_ENUM;
	common->event_type = HID_USAGE_SENSOR_EVENT_DATA_UPDATED_ENUM;
}

static int get_accel_input_report(int report_id, u8 *buf, size_t len,
				  u32 *cpu_addr)
{
	size_t size;
	struct accel3_input_report acc_input;

	get_common_inputs(&acc_input.common, report_id);
	acc_input.accel_x = (int)cpu_addr[0] / FIRMWARE_MULTIPLIER;
	acc_input.accel_y = (int)cpu_addr[1] / FIRMWARE_MULTIPLIER;
	acc_input.accel_z = (int)cpu_addr[2] / FIRMWARE_MULTIPLIER;
	acc_input.shake_detection = (int)cpu_addr[3] / FIRMWARE_MULTIPLIER;
	size = sizeof(acc_input);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &acc_input, size);
	return size;
}

static int get_gyro_input_report(int report_id, u8 *buf, size_t len,
				 u32 *cpu_addr)
{
	size_t size;
	struct gyro_input_report gyro_input;

	get_common_inputs(&gyro_input.common, report_id);
	gyro_input.angle_x = (int)cpu_addr[0] / FIRMWARE_MULTIPLIER;
	gyro_input.angle_y = (int)cpu_addr[1] / FIRMWARE_MULTIPLIER;
	gyro_input.angle_z = (int)cpu_addr[2] / FIRMWARE_MULTIPLIER;
	size = sizeof(gyro_input);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &gyro_input, size);
	return size;
}

static int get_mag_input_report(int report_id, u8 *buf, size_t len,
				u32 *cpu_addr)
{
	size_t size;
	struct magno_input_report magno_input;

	get_common_inputs(&magno_input.common, report_id);
	magno_input.flux_x = (int)cpu_addr[0] / FIRMWARE_MULTIPLIER;
	magno_input.flux_y = (int)cpu_addr[1] / FIRMWARE_MULTIPLIER;
	magno_input.flux_z = (int)cpu_addr[2] / FIRMWARE_MULTIPLIER;
	magno_input.accuracy = (u16)cpu_addr[3] / FIRMWARE_MULTIPLIER;
	size = sizeof(magno_input);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &magno_input, size);
	return size;
}

static int get_als_input_report(int report_id, u8 *buf, size_t len,
				u32 *cpu_addr)
{
	size_t size;
	struct als_input_report als_input;

	get_common_inputs(&als_input.common, report_id);
	als_input.illuminance = (int)cpu_addr[0] / FIRMWARE_MULTIPLIER;
	size = sizeof(als_input);

	if (size > len)
		return -ENOMEM;

	memcpy(buf, &als_input, size);
	return size;
}

/**
 * get_feature_report - Writes an input report and returns its size.
 * @sensor_idx:		The sensor index
 * @report_id:		The report id
 * @buf:		The feature report buffer
 * @cpu_addr:		The DMA mapped CPU address
 *
 * Returns the size on success or < 0 on errors.
 */
int get_input_report(enum sensor_idx sensor_idx, int report_id, u8 *buf,
		     size_t len, u32 *cpu_addr)
{
	if (!buf)
		return -ENOBUFS;

	if (!cpu_addr)
		return -EIO;

	switch (sensor_idx) {
	case ACCEL_IDX:
		return get_accel_input_report(report_id, buf, len, cpu_addr);
	case GYRO_IDX:
		return get_gyro_input_report(report_id, buf, len, cpu_addr);
	case MAG_IDX:
		return get_mag_input_report(report_id, buf, len, cpu_addr);
	case ALS_IDX:
		return get_als_input_report(report_id, buf, len, cpu_addr);
	default:
		return -EINVAL;
	}
}
