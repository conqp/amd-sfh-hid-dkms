/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HID report and report stuructures and routines
 *
 * Author: Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 */

#ifndef AMD_SFH_HID_REPORTS_H
#define AMD_SFH_HID_REPORTS_H

#include "amd-sfh-pci.h"

/**
 * desc_type - Report descriptor types.
 */
enum desc_type {
	AMD_SFH_DESCRIPTOR,
	AMD_SFH_INPUT_REPORT,
	AMD_SFH_FEATURE_REPORT,
};

struct common_features {
	u8 report_id;
	u8 connection_type;
	u8 report_state;
	u8 power_state;
	u8 sensor_state;
	u32 report_interval;
} __packed;

struct common_inputs {
	u8 report_id;
	u8 sensor_state;
	u8 event_type;
} __packed;

struct accel3_feature_report {
	struct common_features common;
	u16 change_sesnitivity;
	s16 sensitivity_max;
	s16 sensitivity_min;
} __packed;

struct accel3_input_report {
	struct common_inputs common;
	int accel_x;
	int accel_y;
	int accel_z;
	u8 shake_detection;
} __packed;

struct gyro_feature_report {
	struct common_features common;
	u16 change_sesnitivity;
	s16 sensitivity_max;
	s16 sensitivity_min;
} __packed;

struct gyro_input_report {
	struct common_inputs common;
	int angle_x;
	int angle_y;
	int angle_z;
} __packed;

struct magno_feature_report {
	struct common_features common;
	u16 headingchange_sensitivity;
	s16 heading_min;
	s16 heading_max;
	u16 flux_change_sensitivity;
	s16 flux_min;
	s16 flux_max;
} __packed;

struct magno_input_report {
	struct common_inputs common;
	int flux_x;
	int flux_y;
	int flux_z;
	int accuracy;
} __packed;

struct als_feature_report {
	struct common_features common;
	u16 change_sesnitivity;
	s16 sensitivity_max;
	s16 sensitivity_min;
} __packed;

struct als_input_report {
	struct common_inputs common;
	int illuminance;
} __packed;

int get_report_descriptor(enum sensor_idx sensor_idx, u8 *buf);
int get_descriptor_size(enum sensor_idx sensor_idx, enum desc_type desc_type);
int get_feature_report(enum sensor_idx sensor_idx, int report_id, u8 *buf,
		       size_t len);
int get_input_report(enum sensor_idx sensor_idx, int report_id, u8 *buf,
		     size_t len, u32 *sensor_virt_addr);
#endif
