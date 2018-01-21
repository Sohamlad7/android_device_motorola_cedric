Sensors HAL Architecture
========================

Dynamic HAL Overview
--------------------

The Dynamic sensors HAL can work as a stand-alone sensors HAL, however
typically it us used as a sub-HAL (along with another standard sensors HAL)
under the Multi-HAL (`hardware/libhardware/modules/sensors`).

The Multi-HAL is responsible for providing a translation layer between the
local sub-HAL sensor handles and the global sensor handles that the framework
uses to refer to the sensors. It converts all fields containing sensor handles
either in the `global->local` or the `local->global` direction, depending on
the message/request direction.

At power-up, the multi-HAL will load all the sub-HALs configured in the
`/system/etc/sensors/hals.conf` file, query each for the list of sensors it
supports, and report the union of all the sensors to the framework.

The Dynamic HAL will report a single sensor at power-up: DynamicMetaSensor. All
other dynamic sensors will be reported as events detected by this
DynamicMetaSensor.

The DynamicMetaSensor uses a UeventListener to listen for kernel UEvents that
indicate the addition or removal of a dynamic sensor. For each dynamic IIO
sensor that the kernel adds/removes, the DynamicMetaSensor reports the
addition/removal as a sensor event, and (if it is an addition) instantiates an
IioSensor that the dynamic HAL uses to represent and interact with the new
sensor.

Sensors HAL Architecture
------------------------

The BaseHal class is the parent of both the dynamic and non-dynamic sensors
HAL. It maintains a list of drivers (i.e. sensors) of SensorBase type that it
dispatches framework requests to by mapping the framework sensor handle to a
specific SensorBase object that corresponds to that handle. A SensorBase
derived object can represent a single sensor (as in the case of IioSensor and
DynamicMetaSensor), or multiple sensors (as in the case of HubSensors).

A SensorsModuleT template is parameterized by a BaseHal type (ex: IioHal or
SensorsPollContext) and provides the `HAL_MODULE_INFO_SYM` symbol which the
Android HAL first looks for to interact with various HALs. The SensorModuleT
defers most of its operations to the BaseHal that it is parameterized with.

Dynamic Sensors
---------------

The only dynamic sensors currently supported are the ones exposed by the kernel
through `sysfs` as IIO devices (see the [IIO interface](@ref doxygen/IIO.md)
documentation or the [Kernel IIO SysFS ABI
Docs](https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-bus-iio) for
more details).

The IioSensor class uses `libiio` (repo: `motorola/external/libiio`, or
https://github.com/analogdevicesinc/libiio) to interface with the `sysfs` and
control the underlying sensor.

The attributes exposed by the IIO sensors are defined by the
[GreyBus interface](@ref doxygen/GreyBus.md) (or see the
[GreyBus Specification](https://drive.google.com/open?id=0B4x0z2O_ateaZUx4MWU0SXZhOTg)
in section 11.2: "Sensors-Ext Protocol") and implemented by the
`/system/lib/modules/gb-sensors_ext.ko` kernel module.

The kernel module source code (`motorola/external/greybus`) is split into
`sensors_ext.c` which handles the interaction with the MOD using the GreyBus
Specification, and `sensors_ext_iio.c` which handles the IIO interaction.

Testing Dynamic Sensors
-----------------------

See the [Dynamic Sensor Tests](@ref doxygen/DynamicSensorTests.md) page for
more details. This document is also available in
[GDrive](https://drive.google.com/open?id=0B4x0z2O_ateaV2E3LVVCVGx3aVk).

