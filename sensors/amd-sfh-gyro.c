/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * AMD Sensor Fusion Hub gyroscope functions
 *
 * Authors:	Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 * 		Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/hid.h>
#include <linux/types.h>

#include "amd-sfh-sensors.h"

struct feature_report {
	struct common_features common;
	u16 change_sesnitivity;
	s16 sensitivity_max;
	s16 sensitivity_min;
} __packed;

struct input_report {
	struct common_inputs common;
	int angle_x;
	int angle_y;
	int angle_z;
} __packed;

static u8 report_descriptor[] = {
0x05, 0x20,		/* Usage page */
0x09, 0x76,		/* Motion type Gyro3D */
0xA1, 0x00,		/* HID Collection (Physical) */

0x85, 2,		/* HID  Report ID */
0x05, 0x20,		/* HID usage page sensor */
0x0A, 0x09, 0x03,	/* Sensor property and sensor connection type */
0x15, 0,		/* HID logical MIN_8(0) */
0x25, 2,		/* HID logical MAX_8(2) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count(1) */
0xA1, 0x02,		/* HID collection (logical) */
0x0A, 0x30, 0x08,	/* Sensor property connection type intergated sel */
0x0A, 0x31, 0x08,	/* Sensor property connection type attached sel */
0x0A, 0x32, 0x08,	/* Sensor property connection type external sel */
0xB1, 0x00,		/* HID feature (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x16, 0x03,	/* HID usage sensor property reporting state */
0x15, 0,		/* HID logical Min_8(0) */
0x25, 5,		/* HID logical Max_8(5) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count(1) */
0xA1, 0x02,		/* HID collection(logical) */
0x0A, 0x40, 0x08,	/* Sensor reporting state no events sel */
0x0A, 0x41, 0x08,	/* Sensor reporting state all events sel */
0x0A, 0x42, 0x08,	/* Sensor reporting state threshold events sel */
0x0A, 0x43, 0x08,	/* Sensor reporting state no events wake sel */
0x0A, 0x44, 0x08,	/* Sensor reporting state all events wake sel */
0x0A, 0x45, 0x08,	/* Sensor reporting state threshold events wake sel */
0xB1, 0x00,		/* HID feature (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x19, 0x03,	/* HID usage sensor property power state */
0x15, 0,		/* HID logical Min_8(0) */
0x25, 5,		/* HID logical Max_8(5) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count(1) */
0xA1, 0x02,		/* HID collection(logical) */
0x0A, 0x50, 0x08,	/* Sensor  power state undefined sel */
0x0A, 0x51, 0x08,	/* Sensor  power state D0 full power  sel */
0x0A, 0x52, 0x08,	/* Sensor  power state D1 low power sel */
0x0A, 0x53, 0x08,	/* Sensor  power state D2 standby with wake sel */
0x0A, 0x54, 0x08,	/* Sensor  power state D3 sleep with wake  sel */
0x0A, 0x55, 0x08,	/* Sensor  power state D4 power off sel */
0xB1, 0x00,		/* HID feature (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x01, 0x02,	/* HID usage sensor state */
0x15, 0,		/* HID logical Min_8(0) */
0x25, 6,		/* HID logical Max_8(6) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count(1) */
0xA1, 0x02,		/* HID collection(logical) */
0x0A, 0x00, 0x08,	/* HID usage sensor state unknown sel */
0x0A, 0x01, 0x08,	/* HID usage sensor state ready sel */
0x0A, 0x02, 0x08,	/* HID usage sensor state not available sel */
0x0A, 0x03, 0x08,	/* HID usage sensor state no data sel */
0x0A, 0x04, 0x08,	/* HID usage sensor state initializing sel */
0x0A, 0x05, 0x08,	/* HID usage sensor state access denied sel */
0x0A, 0x06, 0x08,	/* HID usage sensor state error sel */
0xB1, 0x00,		/* HID feature (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x0E, 0x03,	/* HID usage sensor property report interval */
0x15, 0,		/* HID logical Min_8(0) */
0x27, 0xFF, 0xFF, 0xFF, 0xFF,	/* HID logical Max_32 */

0x75, 32,		/* HID report size(32) */
0x95, 1,		/* HID report count(1) */
0x55, 0,		/* HID unit exponent(0) */
0xB1, 0x02,		/* HID feature (Data_Arr_Abs) */
0x0A, 0x56, 0x14,	/* Angular velocity and mod change sensitivity ABS)*/

0x15, 0,		/* HID logical Min_8(0) */
0x26, 0xFF, 0xFF,	/* HID logical Max_16(0xFF,0xFF) */

0x75, 16,		/* HID report size(16) */
0x95, 1,		/* HID report count(1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0xB1, 0x02,		/* HID feature (Data_Arr_Abs) */
0x0A, 0x56, 0x24,	/* Sensor data (motion angular velocity and mod max) */

0x16, 0x01, 0x80,	/* HID logical Min_16(0x01,0x80) */

0x26, 0xFF, 0x7F,	/* HID logical Max_16(0xFF,0x7F) */

0x75, 16,		/* HID report size(16) */
0x95, 1,		/* HID report count(1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0xB1, 0x02,		/* HID feature (Data_Arr_Abs) */
0x0A, 0x56, 0x34,	/* HID usage sensor data (motion accel and mod min) */

0x16, 0x01, 0x80,	/* HID logical Min_16(0x01,0x80) */

0x26, 0xFF, 0x7F,	/* HID logical Max_16(0xFF,0x7F) */

0x75, 16,		/* HID report size(16) */
0x95, 1,		/* HID report count(1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0xB1, 0x02,		/* HID feature (Data_Arr_Abs) */

//Input reports(transmit)
0x05, 0x20,		/* HID usage page sensors */
0x0A, 0x01, 0x02,	/* HID usage sensor state */
0x15, 0,		/* HID logical Min_8(0) */
0x25, 6,		/* HID logical Max_8(6) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count (1) */
0xA1, 0x02,		/* HID end collection (logical) */
0x0A, 0x00, 0x08,	/* HID usage sensor state unknown sel */
0x0A, 0x01, 0x08,	/* HID usage sensor state ready sel */
0x0A, 0x02, 0x08,	/* HID usage sensor state not available sel */
0x0A, 0x03, 0x08,	/* HID usage sensor state no data sel */
0x0A, 0x04, 0x08,	/* HID usage sensor state initializing sel */
0x0A, 0x05, 0x08,	/* HID usage sensor state access denied sel */
0x0A, 0x06, 0x08,	/* HID usage sensor state error sel */
0X81, 0x00,		/* HID Input (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x02, 0x02,	/* HID usage sensor event */
0x15, 0,		/* HID logical Min_8(0) */
0x25, 5,		/* HID logical Max_8(5) */
0x75, 8,		/* HID report size(8) */
0x95, 1,		/* HID report count (1) */
0xA1, 0x02,		/* HID end collection (logical) */
0x0A, 0x10, 0x08,	/* HID usage sensor event unknown sel */
0x0A, 0x11, 0x08,	/* HID usage sensor event state changed sel */
0x0A, 0x12, 0x08,	/* HID usage sensor event property changed sel */
0x0A, 0x13, 0x08,	/* HID usage sensor event data updated sel */
0x0A, 0x14, 0x08,	/* HID usage sensor event poll response sel */
0x0A, 0x15, 0x08,	/* HID usage sensor event change sensitivity sel */
0X81, 0x00,		/* HID Input (Data_Arr_Abs) */
0xC0,			/* HID end collection */
0x0A, 0x57, 0x04,	/* Sensor data motion Angular velocity  X axis */
0x17, 0x00, 0x00, 0x01, 0x80,	/* HID logical Min_32 */

0x27, 0xFF, 0xFF, 0xFF, 0x7F,	/* HID logical Max_32 */

0x75, 32,		/* HID report size(32) */
0x95, 1,		/* HID report count (1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0X81, 0x02,		/* HID Input (Data_Arr_Abs) */
0x0A, 0x58, 0x04,	/* Sensor data motion Angular velocity  Y axis */
0x17, 0x00, 0x00, 0x01, 0x80, /* HID logical Min_32 */

0x27, 0xFF, 0xFF, 0xFF, 0x7F, /* HID logical Max_32 */

0x75, 32,		/* HID report size(32) */
0x95, 1,		/* HID report count (1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0X81, 0x02,		/* HID Input (Data_Arr_Abs) */
0x0A, 0x59, 0x04,	/* Sensor data motion Angular velocity  Z axis */
0x17, 0x00, 0x00, 0x01, 0x80, /* HID logical Min_32 */

0x27, 0xFF, 0xFF, 0xFF, 0x7F, /* HID logical Max_32 */

0x75, 32,		/* HID report size(32) */
0x95, 1,		/* HID report count (1) */
0x55, 0x0E,		/* HID unit exponent(0x0E) */
0X81, 0x02,		/* HID Input (Data_Arr_Abs) */

0xC0,			/* HID end collection */
};

/**
 * get_gyro_feature_report - Get gyroscope feature report.
 * @reportnum:		Report number
 * @buf:		Report buffer
 * @len:		Size of the report buffer
 *
 * Writes a feature report for the gyroscope to the report buffer.
 *
 * Returns the amout of bytes written on success or < zero on errors.
 */
int get_gyro_feature_report(int reportnum, u8 *buf, size_t len)
{
	struct feature_report report;

	report.change_sesnitivity = AMD_SFH_DEFAULT_SENSITIVITY;
	report.sensitivity_min = AMD_SFH_DEFAULT_MIN_VALUE;
	report.sensitivity_max = AMD_SFH_DEFAULT_MAX_VALUE;
	set_common_features(&report.common, reportnum);

	memcpy(buf, &report, len);
	return len;
}

/**
 * get_gyro_input_report - Get gyroscope input report.
 * @reportnum:		Report number
 * @buf:		Report buffer
 * @len:		Size of the report buffer
 * @cpu_addr:		DMA-mapped CPU address
 *
 * Writes a input report for the gyroscope to the report buffer.
 *
 * Returns the amout of bytes written on success or < zero on errors.
 */
int get_gyro_input_report(int reportnum, u8 *buf, size_t len, u32 *cpu_addr)
{
	struct input_report report;

	if (!cpu_addr)
		return -EIO;

	report.angle_x = (int)cpu_addr[0] / AMD_SFH_FW_MUL;
	report.angle_y = (int)cpu_addr[1] / AMD_SFH_FW_MUL;
	report.angle_z = (int)cpu_addr[2] / AMD_SFH_FW_MUL;
	set_common_inputs(&report.common, reportnum);

	memcpy(buf, &report, len);
	return len;
}

/**
 * parse_gyro_descriptor - Parse the HID descriptor for the gyroscope.
 * @hid:	HID device
 *
 * This function gets called during call to hid_add_device().
 *
 * Returns 0 on success and non-zero on errors.
 */
int parse_gyro_descriptor(struct hid_device *hid)
{
	return hid_parse_report(hid, report_descriptor,
				sizeof(report_descriptor));
}
