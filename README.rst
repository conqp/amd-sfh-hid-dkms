=======================
Kernel drivers: amd-sfh
=======================

Supported adapters:
  * AMD Sensor Fusion Hub PCIe interface

Datasheet: not publicly available.

Authors:
        - Shyam Sundar S K <Shyam-sundar.S-k@amd.com>
        - Nehal Bakulchandra Shah <Nehal-bakulchandra.Shah@amd.com>
        - Sandeep Singh <sandeep.singh@amd.com>
        - Richard Neumann <mail@richard-neumann.de>

Description
===========
The AMD Sensor Fushion Hub (SFH) is part of a SOC on Ryzen-based platforms.
The SFH uses HID over PCIe bus. In terms of architecture it much resmebles the ISH.
However the major difference is, that currently HID reports
are being generated within the kernel driver.

Block Diagram
-------------
.. code-block:: none

    +-------------------------------+
    |  HID User Space Applications  |
    +-------------------------------+
    =================================
    +-------------------------------+
    |      HID low-level driver     |
    |   with HID report generator   |
    +-------------------------------+

    +-------------------------------+
    |     HID client interface      |
    +-------------------------------+

    +-------------------------------+
    |      AMD SFH PCIe driver      |
    +-------------------------------+
    =================================
    +-------------------------------+
    |       SFH MP2 Processor       |
    +-------------------------------+

HID low-level driver
--------------------
The driver is conceived in a multi-layer architecture.
The level closest to the applications is the HID low-level (LL) driver,
which implements the functions defined by the hid-core API to manage the
respective HID devices and process reports.
Therefor, the HID-LL-driver starts and stops the sensors as needed by invoking
the exposed functions from the PCI driver (see below) and creates DMA mappings
to access the DRAM of the PCI device to retrieve feature and input reports
from it.

HID client interface
--------------------
The aforementioned HID devices are being managed, i.e. created on probing and
destroyed on removing, by the client interface used by the PCI driver.
It determines the HID devices to be created on startup using the connected
sensors bitmask retrieved by invoking the respective function of the PCI driver.

PCI device driver (`amd-sfh`)
---------------------------------
The PCI driver is responsible for making all transaction with the chip's
firmware over PCI-e.
The sensors are being started and stopped respectively by writing commands
and, where applicable, DRAM addresses to certain device registers.
The sensor's input report data can then be accessed by accessing the DRAM
through DMA-mapped virtual addresses. Commands are sent to the device using C2P
mail box registers. These C2P registers are mapped in PCIe address space.
Writing into the device message registers generates interrupts. The device's
firmware uses DRAM interface registers to indirectly access DRAM memory. It is
recommended to always write a minimum of 32 bytes into the DRAM.

Driver loading
--------------

+========================+======================+
|       PCI driver       | HID low-level driver |
+===============================================+
| Loaded at boot time if | Used by spawned HIDs |
| device is present.     |                      |
+------------------------+----------------------+

Data flow table
---------------
.. code-block:: none

                                                 +===============================================+
    +============+        Get sensor mask        |             HID client interface              |
    | PCI driver | <---------------------------- +===============================================+
    +============+    of available HID devices   | * Probe HID devices according to sensor mask. |
          ^                                      | * Start periodical polling from DRAM.         |
          |                                      +-----------------------------------------------+
 Start / stop sensor on                                                 |
 respective HID requsts.                                                |
          |                +==============================+             |
          |                |        HID ll-driver         |             |
          +--------------- +==============================+ <-----------+
                           | Provide reports as requested |
                           | by hid-code.                 |
                           +------------------------------+

Quirks
------
On some systems, the sensor hub has not been programmed with information about
the sensors active on the device. This would result in no sensors being
activated and no HID devices being spawned by the driver.
The driver alrady has some quirks for such devices, that automatically
compensate for this by DMI matching and returning an appropriate sensor mask
for the respective device.
You may also manually activate the respective sensors, byloading the module
`amd-sfh` with the kernel parameter `sensor_mask=<int>`.
Available sensors are:

+----------------------+----------+
|        sensor        |   mask   |
+======================+==========+
| accelerometer        |  BIT(0)  |
+----------------------+----------+
| gyroscope            |  BIT(1)  |
+----------------------+----------+
| magnetometer         |  BIT(2)  |
+----------------------+----------+
| ambient light sensor |  BIT(19) |
+----------------------+----------+

To enable e.g. only the accelerometer:

    $ cat /etc/modprobe.d/amd_sfh.conf
    options amd_sfh sensor_mask=1
