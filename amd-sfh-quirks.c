// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub quirks
 *
 * Authors: Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/dmi.h>

#include "amd-sfh-plat.h"
#include "amd-sfh-quirks.h"

/**
 * DMI match for HP ENVY x360 convertibles, which do not
 * have information about the sensor mask in the P2C registers.
 */
static const struct dmi_system_id hp_envy_x360[] = {
	{
		.ident = "HP ENVY x360 Convertible 13-ag0xxx",
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "HP"),
			DMI_MATCH(DMI_BOARD_NAME, "8496"),
			DMI_MATCH(DMI_BOARD_VERSION, "92.48"),
		},
	},
	{
		.ident = "HP ENVY x360 Convertible 15-cp0xxx",
		.matches = {
			DMI_MATCH(DMI_BOARD_VENDOR, "HP"),
			DMI_MATCH(DMI_BOARD_NAME, "8497"),
			DMI_MATCH(DMI_BOARD_VERSION, "92.48"),
		},
	},
	{ }
};

/**
 * Returns the sensor mask for hardwere, on which the
 * sensor mask is not written into the P2C registers.
 *
 * Returns the sensor mask or zero per default.
 */
uint amd_sfh_quirks_get_sensor_mask(void) {
	if (dmi_check_system(hp_envy_x360)) {
		return ACCEL_MASK + MAGNO_MASK;
	}

	return 0;
}
