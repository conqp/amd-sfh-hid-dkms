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
 * struct amd_sfh_common_features - Features common to all sensors.
 * @report_id:		The report number
 * @connection_type:	The connection type
 * @report_state:	The state of the report
 * @power_state:	The power state of the deivce
 * @sensor_state:	The state of the sensor
 * @report_interval	The interval between reports
 */
struct amd_sfh_common_features {
	u8 report_id;
	u8 connection_type;
	u8 report_state;
	u8 power_state;
	u8 sensor_state;
	u32 report_interval;
} __packed;

/**
 * struct amd_sfh_common_inputs - Input data common to all sensors
 * @report_id:		The report number
 * @sensor_state:	The state of the sensor
 * @event_type:		The type of event
 */
struct amd_sfh_common_inputs {
	u8 report_id;
	u8 sensor_state;
	u8 event_type;
} __packed;

enum amd_sfh_sensor_state {
	AMD_SFH_SENSOR_READY = 0x02,
	AMD_SFH_SENSOR_INITIALIZING = 0x05,
};

void amd_sfh_set_common_features(struct amd_sfh_common_features *common,
				 int report_id);
void amd_sfh_set_common_inputs(struct amd_sfh_common_inputs *common,
			       int report_id);
#endif
