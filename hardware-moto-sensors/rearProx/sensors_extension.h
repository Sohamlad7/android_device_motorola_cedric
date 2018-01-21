/*--------------------------------------------------------------------------
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#ifndef SENSORS_EXTENTION_H
#define SENSORS_EXTENTION_H

#include <hardware/sensors.h>
#include <hardware/hardware.h>

#define CMD_W_L_MASK 0x00
#define CMD_W_H_MASK 0x10
#define CMD_W_H_L    0x10
#define DATA_H_MASK    0xFFFF0000
#define DATA_L_MASK    0x0000FFFF

/*
 *sensor calibrate command for sensors axis threshold or factor
 *AXIS_X,AXIS_Y,AXIS_Z,AXIS_XYZ used like accelerometer,magnetometer
 *gyroscope etc.AXIS_THRESHOLD_H AXIS,THRESHOLD_L is used to calibrate
 *proximity threshold,AXIS_BIAS calibrate the noise of proximity sensor.
 *AXIS_FACTOR is used to calibrate light sensor fatcor of the real data
 *AXIS_OFFSET is used to calibrate light sensor offset of the real data
 */
#define AXIS_X        0
#define AXIS_Y        1
#define AXIS_Z        2
#define AXIS_XYZ    3

#define AXIS_THRESHOLD_H    0
#define AXIS_THRESHOLD_L    1
#define AXIS_BIAS        2

#define AXIS_FACTOR        0
#define AXIS_OFFSET        1

#define AXIS_SHIFT    17
#define SAVE_SHIFT    16
#define CMD_SHIFT    16

#define LENGTH        33 /*one cal_result and separator length*/
__BEGIN_DECLS


/*
 *CMD_DO_CAL when do sensor calibrate must use this
 *AXIS_X,AXIS_Y,AXIS_Z write axis params to driver like accelerometer
 *magnetometer,gyroscope etc.CMD_W_THRESHOLD_H,CMD_W_THRESHOLD_L,CMD_W_BIAS
 *write theshold and bias params to proximity driver.CMD_W_FACTOR,CMD_W_OFFSET
 *write factor and offset params to light sensor driver.
 */
enum {
    CMD_DO_CAL = 0x0,
    CMD_W_OFFSET_X,
    CMD_W_OFFSET_Y,
    CMD_W_OFFSET_Z,
    CMD_W_THRESHOLD_H,
    CMD_W_THRESHOLD_L,
    CMD_W_BIAS,
    CMD_W_OFFSET,
    CMD_W_FACTOR,
    CMD_W_RANGE,
    CMD_COMPLETE,
    CMD_COUNT
};

/*
 *macro SET_CMD_H() write sensors params high 16 bits.
 *SET_CMD_L() write sensors params low 16 bits.
 *CMD_CAL()calibrate command
 */
#define SET_CMD_H(param, CMDS) \
    (((param) & DATA_H_MASK) | CMDS \
    | (CMD_W_H_L & CMD_W_H_MASK))

#define SET_CMD_L(param, CMDS) \
    (((param) & DATA_L_MASK) << CMD_SHIFT | CMDS \
     | (CMD_W_H_L & CMD_W_L_MASK))

#define CMD_CAL(axis, apply_now) \
    ((axis) << AXIS_SHIFT | (apply_now)<< SAVE_SHIFT | CMD_DO_CAL)

struct sensors_poll_device_1_ext_t {
    union {

        struct sensors_poll_device_1 aosp;
        struct {
            struct hw_device_t common;
            int (*activate)(struct sensors_poll_device_t *dev,
                    int handle, int enabled);
            int (*setDelay)(struct sensors_poll_device_t *dev,
                    int handle, int64_t period_ns);
            int (*poll)(struct sensors_poll_device_t *dev,
                    sensors_event_t* data, int count);
            int (*batch)(struct sensors_poll_device_1* dev,
                    int handle, int flags, int64_t period_ns, int64_t timeout);
            int (*flush)(struct sensors_poll_device_1* dev, int handle);
            void (*reserved_procs[8])(void);
        };
    };
};

struct cal_result_t {
    union{

        struct {
            int offset_x;    /*axis offset of x axis*/
            int offset_y;    /*axis offset of x axis*/
            int offset_z;    /*axis offset of x axis*/
        };
        struct {
            int threshold_h; /*proximity threshold_h*/
            int threshold_l; /*proximity threshold_l*/
            int bias;    /*proximity measure data noise*/
        };
        int offset[3];
    };
    int factor; /*light sensor factor for real ligt strength*/
    int range;
    int *node;
};

static inline int sensors_open_ext(const struct hw_module_t* module,
        struct sensors_poll_device_1_ext_t** device)
{
    return module->methods->open(module,
            SENSORS_HARDWARE_POLL, (struct hw_device_t**)device);
}

static inline int sensors_close_ext(struct sensors_poll_device_1_ext_t* device)
{
    return device->common.close(&device->common);
}

__END_DECLS
#endif


