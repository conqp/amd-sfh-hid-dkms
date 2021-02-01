/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * AMD Sensor Fusion Hub common sensor functions
 *
 * Author:	Richard Neumann <mail@richard-neumann.de>
 */

#include "amd-sfh-sensors.h"

/**
 * set_common_features - Sets common values on feature reports.
 * @common:	Pointer to the common features struct
 * @reportnum:	The report number
 */
void set_common_features(struct common_features *common, int report_id)
{
	common->report_id = report_id;
	common->connection_type = AMD_SFH_CONNECTION_TYPE;
	common->report_state = AMD_SFH_REPORT_STATE;
	common->power_state = AMD_SFH_POWER_STATE;
	common->sensor_state = AMD_SFH_SENSOR_INITIALIZING;
	common->report_interval =  AMD_SFH_REPORT_INTERVAL;
}

/**
 * set_common_inputs - Sets common values on input reports.
 * @common:	Pointer to the common inputs struct
 * @reportnum:	The report number
 */
void set_common_inputs(struct common_inputs *common, int report_id)
{
	common->report_id = report_id;
	common->sensor_state = AMD_SFH_SENSOR_READY;
	common->event_type = AMD_SFH_EVENT_TYPE;
}
