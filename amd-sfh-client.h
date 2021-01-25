/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 *  AMD Sensor Fusion Hub HID client interface
 *
 *  Authors: Nehal Bakulchandra Shah <Nehal-Bakulchandra.Shah@amd.com>
 *           Richard Neumann <mail@richard-neumann.de>
 */

#ifndef AMD_SFH_CLIENT_H
#define AMD_SFH_CLIENT_H

#include <linux/pci.h>

#include "amd-sfh.h"

int amd_sfh_client_init(struct amd_sfh_data *privdata);
int amd_sfh_client_deinit(struct amd_sfh_data *privdata);

#endif
