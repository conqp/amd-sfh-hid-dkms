/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * AMD Sensor Fusion Hub HID descriptor definitions
 *
 * Authors:	Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 * 		Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_HID_DESCRIPTORS_H
#define AMD_SFH_HID_DESCRIPTORS_H

#include <linux/types.h>

extern const u8 accel3_report_descriptor[];
extern const size_t accel3_report_descriptor_size;
extern const u8 gyro3_report_descriptor[];
extern const size_t gyro3_report_descriptor_size;
extern const u8 magno_report_descriptor[];
extern const size_t magno_report_descriptor_size;
extern const u8 als_report_descriptor[];
extern const size_t als_report_descriptor_size;
#endif
