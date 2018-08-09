/*
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 *
 * This file is released under the GPLv2
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/memory.h>

#include "base.h"

/**
 * driver_init - initialize driver model.
 *
 * Call the driver model init functions to initialize their
 * subsystems. Called early from init/main.c.
 */
void __init driver_init(void)
{
	/* These are the core pieces */
	devices_init();// sys/devices  sys/dev   sys/dev/block sys/dev/char
	buses_init();  // sys/bus
	classes_init();// sys/class
	firmware_init();//sys/firmware
	hypervisor_init();//sys/hypervisor

	/* These are also core pieces, but must come after the
	 * core core pieces.
	 */
	platform_bus_init(); //sys/devices/platform
	system_bus_init();   //sys/devices/system
	cpu_dev_init();      //sys/devices/system/cpu
	memory_dev_init();   //sys/devices/system/memory
}
