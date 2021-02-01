/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AMD Sensor Fusion Hub accelerometer interface
 *
 * Author:	Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_ACCEL_H
#define AMD_SFH_ACCEL_H

#include <linux/hid.h>
#include <linux/types.h>

int get_accel_feature_report(int reportnum, u8 *buf, size_t len);
int get_accel_input_report(int reportnum, u8 *buf, size_t len, u32 *cpu_addr);
int parse_accel_descriptor(struct hid_device *hid);

#endif
