/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * AMD Sensor Fusion Hub lid switch functions
 *
 * Authors:	Ivan Dovgal
 * 		Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/hid.h>
#include <linux/types.h>

#include "amd-sfh-sensors.h"

struct feature_report {
	struct common_features common;
} __packed;

struct input_report {
	struct common_inputs common;
	u8 state;
} __packed;

static u8 report_descriptor[] = {
0x06, 0x43, 0xFF,  // Usage Page (Vendor Defined 0xFF43)
0x0A, 0x02, 0x02,  // Usage (0x0202)
0xA1, 0x01,        // Collection (Application)
0x85, 0x11,        //   Report ID (17)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x01,        //   Physical Maximum (1)
0x65, 0x00,        //   Unit (None)
0x55, 0x00,        //   Unit Exponent (0)
0x75, 0x01,        //   Report Size (1)
0x95, 0x98,        //   Report Count (-104)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC1, 0x00,        // End Collection
};

/**
 * get_lid_feature_report - Get lid switch feature report.
 * @reportnum:		Report number
 * @buf:		Report buffer
 * @size:		Size of the report buffer
 *
 * Writes a feature report for the lid switch to the report buffer.
 *
 * Returns the amout of bytes written on success or < zero on errors.
 */
int get_lid_feature_report(int reportnum, u8 *buf, size_t len)
{
	struct feature_report report;

	set_common_features(&report.common, reportnum);

	memcpy(buf, &report, len);
	return len;
}

/**
 * get_lid_input_report - Get lid switch input report.
 * @reportnum:		Report number
 * @buf:		Report buffer
 * @len:		Size of the report buffer
 * @cpu_addr:		DMA-mapped CPU address
 *
 * Writes an input report for the lid switch to the report buffer.
 *
 * Returns the amout of bytes written on success or < zero on errors.
 */
int get_lid_input_report(int reportnum, u8 *buf, size_t len, u32 *cpu_addr)
{
	struct input_report report;

	if (!cpu_addr)
		return -EIO;

	report.state = (int)cpu_addr[0] / AMD_SFH_FW_MUL;
	set_common_inputs(&report.common, reportnum);

	memcpy(buf, &report, len);
	return len;
}

/**
 * parse_lid_descriptor - Parse the HID descriptor for the lid switch.
 * @hid:	HID device
 *
 * This function gets called during call to hid_add_device().
 *
 * Returns 0 on success and non-zero on errors.
 */
int parse_lid_descriptor(struct hid_device *hid)
{
	return hid_parse_report(hid, report_descriptor,
				sizeof(report_descriptor));
}
