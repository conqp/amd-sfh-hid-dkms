/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AMD Sensor Fusion common sensors interface
 *
 * Authors:	Sandeep Singh <sandeep.singh@amd.com>
 *		Nehal Bakulchandra Shah <Nehal-bakulchandra.shah@amd.com>
 *		Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_SENSORS_H
#define AMD_SFH_SENSORS_H

#include <linux/dma-mapping.h>
#include <linux/hid.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#define AMD_SFH_DMA_SIZE		(sizeof(int) * 8)
#define AMD_SFH_FW_MUL			1000
#define AMD_SFH_CONNECTION_TYPE		0x01
#define AMD_SFH_REPORT_STATE		0x41
#define AMD_SFH_POWER_STATE		0x51
#define AMD_SFH_REPORT_INTERVAL		0x50
#define AMD_SFH_EVENT_TYPE		0x04
#define AMD_SFH_DEFAULT_MIN_VALUE	0X7F
#define AMD_SFH_DEFAULT_MAX_VALUE	0x80
#define AMD_SFH_DEFAULT_SENSITIVITY	0x7F

/**
 * struct common_features - Features common to all sensors.
 * @report_id:		Report number
 * @connection_type:	Connection type
 * @report_state:	State of the report
 * @power_state:	Power state of the deivce
 * @sensor_state:	State of the sensor
 * @report_interval	Interval between reports
 */
struct common_features {
	u8 report_id;
	u8 connection_type;
	u8 report_state;
	u8 power_state;
	u8 sensor_state;
	u32 report_interval;
} __packed;

/**
 * struct common_inputs - Input data common to all sensors
 * @report_id:		Report number
 * @sensor_state:	State of the sensor
 * @event_type:		Type of event
 */
struct common_inputs {
	u8 report_id;
	u8 sensor_state;
	u8 event_type;
} __packed;

enum amd_sfh_sensor_state {
	AMD_SFH_SENSOR_READY = 0x02,
	AMD_SFH_SENSOR_INITIALIZING = 0x05,
};

void set_common_features(struct common_features *common, int report_id);
void set_common_inputs(struct common_inputs *common, int report_id);
#endif
