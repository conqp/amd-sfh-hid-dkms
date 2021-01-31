// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 *  AMD Sensor Fusion Hub HID report and descriptor generation
 *
 *  Author: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *          Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "amd-sfh-hid-reports.h"
#include "amd-sfh-report-descriptors.h"

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

int get_accel_feature_report(int report_id, u8 *buf, size_t len)
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

int get_gyro_feature_report(int report_id, u8 *buf, size_t len)
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

int get_mag_feature_report(int report_id, u8 *buf, size_t len)
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

int get_als_feature_report(int report_id, u8 *buf, size_t len)
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

static void get_common_inputs(struct common_inputs *common, int report_id)
{
	common->report_id = report_id;
	common->sensor_state = HID_USAGE_SENSOR_STATE_READY_ENUM;
	common->event_type = HID_USAGE_SENSOR_EVENT_DATA_UPDATED_ENUM;
}

int get_accel_input_report(int report_id, u8 *buf, size_t len, u32 *cpu_addr)
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

int get_gyro_input_report(int report_id, u8 *buf, size_t len, u32 *cpu_addr)
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

int get_mag_input_report(int report_id, u8 *buf, size_t len, u32 *cpu_addr)
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

int get_als_input_report(int report_id, u8 *buf, size_t len, u32 *cpu_addr)
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
