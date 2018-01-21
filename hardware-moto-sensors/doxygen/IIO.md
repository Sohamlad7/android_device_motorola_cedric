IIO
===

Overview of IIO
---------------

The Industrial I/O subsystem is intended to provide support for devices
that in some sense are analog to digital converters (ADCs). As many
actual devices combine some ADCs with digital to analog converters
(DACs) that functionality is also supported.

The aim is to fill the gap between the somewhat similar hwmon and
input subsystems.  Hwmon is very much directed at low sample rate
sensors used in applications such as fan speed control and temperature
measurement.  Input is, as its name suggests focused on input
devices. In some cases there is considerable overlap between these and
IIO.

A typical device falling into this category would be connected via SPI
or I2C.

Functionality of IIO
--------------------

* Basic device registration and handling. This is very similar to
hwmon with simple polled access to device channels via sysfs.

* Event chrdevs.  These are similar to input in that they provide a
route to user space for hardware triggered events. Such events include
threshold detectors, free-fall detectors and more complex action
detection.  The events themselves are currently very simple with
merely an event code and a timestamp.  Any data associated with the
event must be accessed via polling.

Note: A given device may have one or more event channel.  These events are
turned on or off (if possible) via sysfs interfaces.

* Hardware buffer support.  Some recent sensors have included
fifo / ring buffers on the sensor chip.  These greatly reduce the load
on the host CPU by buffering relatively large numbers of data samples
based on an internal sampling clock. Examples include VTI SCA3000
series and Analog Device ADXL345 accelerometers.  Each buffer supports
polling to establish when data is available.

* Trigger and software buffer support. In many data analysis
applications it it useful to be able to capture data based on some
external signal (trigger).  These triggers might be a data ready
signal, a gpio line connected to some external system or an on
processor periodic interrupt.  A single trigger may initialize data
capture or reading from a number of sensors.  These triggers are
used in IIO to fill software buffers acting in a very similar
fashion to the hardware buffers described above.

IIO trigger drivers
-------------------

Many triggers are provided by hardware that will also be registered as
an IIO device.  Whilst this can create device specific complexities
such triggers are registered with the core in the same way as
stand-alone triggers.

LibIIO
------

Libiio is a library that has been developed by Analog Devices to ease the
development of software interfacing Linux Industrial I/O (IIO) devices.

The library abstracts the low-level details of the hardware, and provides a
simple yet complete programming interface that can be used for advanced
projects.

The library is composed by one high-level API, and several backends:

* the “local” backend, which interfaces the Linux kernel through the sysfs
virtual filesystem

* the “network” backend, which interfaces the iiod server through a network
link.

The IIO Daemon (IIOD) server is a good example of an application that uses
libiio. It creates a libiio context that uses the “local” backend, and then
share it on the network to any client application using the “network” backend
of libiio and connected to the server.

SysFS Directory Structure
-------------------------

Here's an example of the `sysfs` structure when a MOD is attach that provides 3
sensors: 2 accelerometers (device 0 and 2) and 1 barometer (device 1).

    .
    |---iio:device0
    | |---buffer
    | | |---enable
    | | |---length
    | |---dev
    | |---events
    | | |---buffer_empty_falling_en
    | | |---buffer_empty_falling_value
    | |---fifo_mec
    | |---fifo_rec
    | |---flags
    | |---flush
    | |---greybus_name
    | |---greybus_name_len
    | |---greybus_type
    | |---greybus_version
    | |---iiodata
    | |---in_accel_x_raw
    | |---in_accel_y_raw
    | |---in_accel_z_raw
    | |---in_offset
    | |---in_sampling_frequency
    | |---in_scale
    | |---max_delay_us
    | |---max_latency_ns
    | |---max_range
    | |---min_delay_us
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---power_uA
    | |---resolution
    | |---scan_elements
    | | |---in_accel_x_en
    | | |---in_accel_x_index
    | | |---in_accel_x_type
    | | |---in_accel_y_en
    | | |---in_accel_y_index
    | | |---in_accel_y_type
    | | |---in_accel_z_en
    | | |---in_accel_z_index
    | | |---in_accel_z_type
    | | |---in_timestamp_en
    | | |---in_timestamp_index
    | | |---in_timestamp_type
    | |---string_type
    | |---string_type_len
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---trigger
    | | |---current_trigger
    | |---uevent
    | |---vendor
    | |---vendor_len
    |
    |---iio:device1
    | |---buffer
    | | |---enable
    | | |---length
    | |---dev
    | |---events
    | | |---buffer_empty_falling_en
    | | |---buffer_empty_falling_value
    | |---fifo_mec
    | |---fifo_rec
    | |---flags
    | |---flush
    | |---greybus_name
    | |---greybus_name_len
    | |---greybus_type
    | |---greybus_version
    | |---iiodata
    | |---in_offset
    | |---in_pressure0_raw
    | |---in_sampling_frequency
    | |---in_scale
    | |---max_delay_us
    | |---max_latency_ns
    | |---max_range
    | |---min_delay_us
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---power_uA
    | |---resolution
    | |---scan_elements
    | | |---in_pressure0_en
    | | |---in_pressure0_index
    | | |---in_pressure0_type
    | | |---in_timestamp_en
    | | |---in_timestamp_index
    | | |---in_timestamp_type
    | |---string_type
    | |---string_type_len
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---trigger
    | | |---current_trigger
    | |---uevent
    | |---vendor
    | |---vendor_len
    |
    |---iio:device2
    | |---buffer
    | | |---enable
    | | |---length
    | |---dev
    | |---events
    | | |---buffer_empty_falling_en
    | | |---buffer_empty_falling_value
    | |---fifo_mec
    | |---fifo_rec
    | |---flags
    | |---flush
    | |---greybus_name
    | |---greybus_name_len
    | |---greybus_type
    | |---greybus_version
    | |---iiodata
    | |---in_accel_x_raw
    | |---in_accel_y_raw
    | |---in_accel_z_raw
    | |---in_offset
    | |---in_sampling_frequency
    | |---in_scale
    | |---max_delay_us
    | |---max_latency_ns
    | |---max_range
    | |---min_delay_us
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---power_uA
    | |---resolution
    | |---scan_elements
    | | |---in_accel_x_en
    | | |---in_accel_x_index
    | | |---in_accel_x_type
    | | |---in_accel_y_en
    | | |---in_accel_y_index
    | | |---in_accel_y_type
    | | |---in_accel_z_en
    | | |---in_accel_z_index
    | | |---in_accel_z_type
    | | |---in_timestamp_en
    | | |---in_timestamp_index
    | | |---in_timestamp_type
    | |---string_type
    | |---string_type_len
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---trigger
    | | |---current_trigger
    | |---uevent
    | |---vendor
    | |---vendor_len
    |
    |---trigger0
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---uevent
    |
    |---trigger1
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---uevent
    |
    |---trigger2
    | |---name
    | |---power
    | | |---autosuspend_delay_ms
    | | |---control
    | | |---runtime_active_time
    | | |---runtime_status
    | | |---runtime_suspended_time
    | |---subsystem
    | | |---drivers
    | | |---drivers_autoprobe
    | | |---drivers_probe
    | | |---uevent
    | |---uevent

Let's go through some of these entries:

main directory contents
-----------------------

    What:           /sys/bus/iio/devices/iio:deviceX
    Description:
                    Hardware chip or device accessed by one communication port.
                    Corresponds to a grouping of sensor channels. X is the IIO
                    index of the device.

    What:           /sys/bus/iio/devices/iio:deviceX/name
    Description:
                    Description of the physical chip / device for device X.
                    Typically a part number.

    What:           /sys/bus/iio/devices/iio:deviceX/in_pressure0_raw
    Description:
                    Raw pressure measurement from channel Y. Units after
                    application of scale and offset are kilopascal.

    What:           /sys/bus/iio/devices/triggerX
    Description:
                    An event driven driver of data capture to an in kernel buffer.
                    May be provided by a device driver that also has an IIO device
                    based on hardware generated events (e.g. data ready) or
                    provided by a separate driver for other hardware (e.g.
                    periodic timer, GPIO or high resolution timer).
                    Contains trigger type specific elements. These do not
                    generalize well and hence are not documented in this file.
                    X is the IIO index of the trigger.

    What:           /sys/bus/iio/devices/iio:deviceX/trigger/current_trigger
    Description:
                    The name of the trigger source being used, as per string given
                    in /sys/class/iio/triggerY/name.

buffer directory contents
-------------------------

    What:           /sys/bus/iio/devices/iio:deviceX/buffer
    Description:
                    Directory of attributes relating to the buffer for the device.

    What:           /sys/bus/iio/devices/iio:deviceX/buffer/length
    Description:
                    Number of scans contained by the buffer.

    What:           /sys/bus/iio/devices/iio:deviceX/buffer/enable
    Description:
                    Actually start the buffer capture up.  Will start trigger
                    if first device and appropriate.

scan_elements directory contents
--------------------------------

    What:           /sys/bus/iio/devices/iio:deviceX/scan_elements
    Description:
                    Directory containing interfaces for elements that will be
                    captured for a single triggered sample set in the buffer.

    What:           /sys/.../iio:deviceX/scan_elements/in_accel_x_en
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_y_en
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_z_en
    What:           /sys/.../iio:deviceX/scan_elements/in_pressure0_en
    Description:
                    Scan element control for triggered data capture.

    What:           /sys/.../iio:deviceX/scan_elements/in_accel_x_type
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_y_type
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_z_type
    What:           /sys/.../iio:deviceX/scan_elements/in_pressure0_type
    Description:
                    Description of the scan element data storage within the buffer
                    and hence the form in which it is read from user-space.
                    Form is [be|le]:[s|u]bits/storagebits[>>shift].
                    be or le specifies big or little endian. s or u specifies if
                    signed (2's complement) or unsigned. bits is the number of bits
                    of data and storagebits is the space (after padding) that it
                    occupies in the buffer. shift if specified, is the shift that
                    needs to be applied prior to masking out unused bits. Some
                    devices put their data in the middle of the transferred elements
                    with additional information on both sides.  Note that some
                    devices will have additional information in the unused bits
                    so to get a clean value, the bits value must be used to mask
                    the buffer output value appropriately.  The storagebits value
                    also specifies the data alignment.  So s48/64>>2 will be a
                    signed 48 bit integer stored in a 64 bit location aligned to
                    a 64 bit boundary. To obtain the clean value, shift right 2
                    and apply a mask to zero the top 16 bits of the result.
                    For other storage combinations this attribute will be extended
                    appropriately.

    What:           /sys/.../iio:deviceX/scan_elements/in_accel_x_index
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_y_index
    What:           /sys/.../iio:deviceX/scan_elements/in_accel_z_index
    What:           /sys/.../iio:deviceX/scan_elements/in_pressure0_index
    Description:
                    A single positive integer specifying the position of this
                    scan element in the buffer. Note these are not dependent on
                    what is enabled and may not be contiguous. Thus for user-space
                    to establish the full layout these must be used in conjunction
                    with all _en attributes to establish which channels are present,
                    and the relevant _type attributes to establish the data storage
                    format.

iio-trigger-sysfs
-----------------

This device is useful for development or testing. It allows the user to have
better control of the event triggering.

    What:           /sys/bus/iio/devices/triggerX/trigger_now
    Description:
                    This file is provided by the iio-trig-sysfs stand-alone trigger
                    driver. Writing this file with any value triggers an event
                    driven driver, associated with this trigger, to capture data
                    into an in kernel buffer. This approach can be valuable during
                    automated testing or in situations, where other trigger methods
                    are not applicable. For example no RTC or spare GPIOs.
                    X is the IIO index of the trigger.

    What:           /sys/bus/iio/devices/triggerX/trigger_poll
    Description:
                    This file is provided by the iio-trig-sysfs stand-alone trigger
                    driver. Writing this file with positive value (in milliseconds)
                    will start periodic event triggering of the driver, associated
                    with this trigger. Writing this file with 0 will stop perioding
                    triggering.
                    X is the IIO index of the trigger.


GreyBus specific entries
------------------------

The following IIO device attributes are be mapped to the corresponding field in
the Greybus Sensor Info Response. See the GreyBus spec for more details on
these fields.

    Field           Sysfs
    ==============  =============================================
    version         /sys/bus/iio/devices/iio:device[n]/version
    type            /sys/bus/iio/devices/iio:device[n]/type
    max_range       /sys/bus/iio/devices/iio:device[n]/max_range
    resolution      /sys/bus/iio/devices/iio:device[n]/resolution
    power           /sys/bus/iio/devices/iio:device[n]/power
    min_delay       /sys/bus/iio/devices/iio:device[n]/min_delay
    max_delay       /sys/bus/iio/devices/iio:device[n]/max_delay
    fifo_rec        /sys/bus/iio/devices/iio:device[n]/fifo_rec
    fifo_mec        /sys/bus/iio/devices/iio:device[n]/fifo_mec
    flags           /sys/bus/iio/devices/iio:device[n]/flags
    name            /sys/bus/iio/devices/iio:device[n]/name
    vendor          /sys/bus/iio/devices/iio:device[n]/vendor
    string_type     /sys/bus/iio/devices/iio:device[n]/string_type

