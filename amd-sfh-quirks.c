// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * AMD Sensor Fusion Hub quirks
 *
 * Authors: Richard Neumann <mail@richard-neumann.de>
 */

#include <linux/dmi.h>
#include <linux/pci.h>

#include "amd-sfh.h"
#include "amd-sfh-quirks.h"

static const struct amd_sfh_quirks hp_envy_x360_quirks = {
	.sensor_mask = ACCEL_MASK | MAGNO_MASK
};

/**
 * DMI match for HP ENVY x360 convertibles, which do not
 * have information about the sensor mask in the P2C registers.
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
 * Returns the sensor mask for hardware, on which the
 * sensor mask is not written into the P2C registers.
 *
 * Returns an appropriate sensor mask or zero per default.
 */
uint amd_sfh_quirks_get_sensor_mask(struct pci_dev *pci_dev)
{
	const struct dmi_system_id *dmi_match;
	const struct amd_sfh_quirks *quirks;

	dmi_match = dmi_first_match(amd_sfh_dmi_quirks);
	if (dmi_match) {
		pci_info(pci_dev, "Detected %s.\n", dmi_match->ident);
		quirks = dmi_match->driver_data;
		return quirks->sensor_mask;
	}

	pci_warn(pci_dev, "No quirks available for this hardware.\n");
	return 0;
}
