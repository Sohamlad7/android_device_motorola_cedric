Sensors HAL
===========

Dynamic Sensors Overview
========================

There are 3 main components to the dynamic sensors:

  1. The [GreyBus interface](@ref doxygen/GreyBus.md) between the phone's
     kernel GreyBus module and the MOD
  2. The [IIO interface](@ref doxygen/IIO.md)  between the kernel GreyBus
     module and user-space
  3. The [IIO HAL](@ref doxygen/HAL.md) (or sub-HAL) interface between the
     kernel `sysfs` and the Android framework.

Each one will be discussed and documented in a separate chapter.
