// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub quirks
 *
 * Author:	Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/dmi.h>

#include "amd-sfh.h"
#include "amd-sfh-quirks.h"

/**
 * Quirks for HP Envy x360 series systems.
 */
static const struct amd_sfh_quirks hp_envy_x360_quirks = {
	.sensor_mask = ACCEL_MASK | MAG_MASK | LID_MASK
};

/**
 * DMI matches for devices that need quirks to work properly.
 */
static const struct dmi_system_id amd_sfh_dmi_quirks[] = {
	{
		.ident = "HP ENVY x360 Convertible 13-ag0xxx",
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "HP"),
			DMI_MATCH(DMI_BOARD_NAME, "8496"),
			DMI_MATCH(DMI_BOARD_VERSION, "92.48"),
		},
		.driver_data = (void *)&hp_envy_x360_quirks
	},
	{
		.ident = "HP ENVY x360 Convertible 15-cp0xxx",
		.matches = {
			DMI_MATCH(DMI_BOARD_VENDOR, "HP"),
			DMI_MATCH(DMI_BOARD_NAME, "8497"),
			DMI_MATCH(DMI_BOARD_VERSION, "92.48"),
		},
		.driver_data = (void *)&hp_envy_x360_quirks
	},
	{ }
};

/**
 * Returns quirks for hardware, where appropriate or NULL per default.
 */
struct amd_sfh_quirks *amd_sfh_get_quirks(void)
{
	const struct dmi_system_id *dmi_match;

	dmi_match = dmi_first_match(amd_sfh_dmi_quirks);
	if (dmi_match)
		return dmi_match->driver_data;

	return NULL;
}
