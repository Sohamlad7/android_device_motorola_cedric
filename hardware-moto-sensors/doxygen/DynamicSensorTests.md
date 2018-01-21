Dynamic Sensor Tests
====================

Prerequisites
=============

* A phone loaded with an Android N or later build. Dynamic sensors are not
  supported on builds before Android N.
* A sensor MOD. Two options:
  * Using the HDK:
    * MODs HDK
    * Raspberry Pi Sense HAT (https://www.adafruit.com/products/2738)
    * Appropriate firmware for the Sense HAT (TBD)
  * Some TBD commercial MOD
* [SensorDebugTool.apk](https://sites.google.com/a/motorola.com/sensorhub/tools-repository)
  (v1.7.1 or later) installed on the phone.
* MDKUtility-playstore-debug.apk installed on the phone

The Sense HAT firmware does not contain a full-blown commercial sensor driver.
It was developed for simple test purposes. As such, some of the standard sensor
functionality may be missing or incomplete. For example, batching may not be
supported, or some sampling rates may not report as expected.

Initial Setup
=============

Using HDK
---------
- Power-up phone
- `adb push nuttx.tftf /sdcard/Download/`
- Verify that all the HDK switches are off except for A1 and B4.
- Plug USB-C charger into the middle port (port 2) to charge the HDK.
- Wait until the HDK is charged. The "Moto Mods Manager" notification will show
  the battery level.
- Set switch 1, 2, and 4 on the HAT adapter board to OFF, and switch 3 to ON.
- Attach the Raspberry Pi Sense HAT to the "HAT Adapter Board".
- Attach the HAT Adapter Board to the HDK MOD
  - The MOD battery will drain as long as the HAT Adapter Board is connected
    to the MOD and B3 is OFF. To prevent the battery from draining, remove the
    adapter board, or set B3 to ON after testing is done and the MOD is detached.
- Attach the HDK MOD to the phone
- Start MDKUtility app
  - In the "Firmware Update" section, click "Select File(s)"
  - From the right menu, select "Show Internal Storage"
  - From the left menu, select "XT1650"
  - Navigate to "Download"
  - Select the "nuttx.tftf" file
  - Click on "Perform Update"

Using TBD commercial MOD
------------------------
- TBD

Tests
=====

Power-up
--------
- Power up phone with no MOD attached
- Launch SensorDebugTool
  - Verify that "Greybus IIO Dynamic MetaSensor" (aka: Dynamic
    MetaSensor) shows up in the list of sensors (typically at the end of
    the list).
- Enable the "Sensor Details" checkbox at the top.
  - Verify that the Dynamic MetaSensor shows: "Is Dynamic: false"
- Register for the Dynamic MetaSensor.
  - Verify that no sensor data is received/displayed
- Un-register for the Dynamic MetaSensor.
  - Verify that nothing unusual happens

Attach
------
- Perform the `Power-up` test
- Attach a sensor MOD
  - If this is the first time the sensor MOD is attached, go through the
    initial MOD setup
  - Verify that at least one new dynamic sensor is shown in the list of
    sensors (the sensor name will have "(Dynamic)" appended).
  - Enable the "Sensor Details" checkbox at the top and verify that the
    newly added sensor shows: "Is Dynamic: true"
  - If the Dynamic MetaSensor is registered, it may show a (bogus) event.
- Register for the new dynamic sensor
  - Verify that the sensor reports data at the requested rate.

Detach
------
- Perform the `Attach` test
- Unregister for the dynamic sensor
  - Verify that the sensor stops reporting data
- Detach the sensor MOD
  - Verify that the dynamic sensor is removed from the list of sensors
    shown in SensorDebugTool
  - Verify that the Dynamic MetaSensor is still present in the list of
    sensors shown in the SensorDebugTool

Detach while running
--------------------
- Perform the `Attach` test
- Detach the sensor MOD without un-registering from the dynamic sensor
  - Verify that the dynamic sensor is removed from the list of sensors
    shown in SensorDebugTool
  - Verify that the Dynamic MetaSensor is still present in the list of
    sensors shown in the SensorDebugTool

Re-attach
---------
- Perform the `Detach while running` test
- Attach the sensor MOD
  - Verify that the dynamic sensor(s) is/are added back to the list.
  - Verify that no sensor events are reported by the dynamic sensors
- Register for the new dynamic sensor
  - Verify that the sensor reports data at the requested rate.
- Detach the sensor MOD without un-registering from the dynamic sensor
  - Verify that the dynamic sensor is removed from the list of sensors
    shown in SensorDebugTool
  - Verify that the Dynamic MetaSensor is still present in the list of
    sensors shown in the SensorDebugTool
- Repeat from step #2 a few more times.
- If the attach/detach can be automated, perform the attach/detach a few
  hundred times and at the end verify that the dynamic sensor is shown in the
  list and reports data when registered.

Time-stamps
-----------
- Can not be tested yet because dynamic sensor data is not yet stamped with
  time-since-boot.
- Attach MOD
- Start recording accel data with SensorLogger from both the external/dynamic
  accelerometer and the internal one.
- Perform some movement of the device (some sharp taps).
- Save the log and extract it from the phone.
- Compare the data collected from the two sensors and verify that the cross
  correlation is at a maximum when the timestamps are aligned.

CTS Verifier
------------
- Install `CTS Verifier 7.0_r0` or later
- Execute the `Dynamic Sensor Discovery Test`
  - Verify that the test fully passes

