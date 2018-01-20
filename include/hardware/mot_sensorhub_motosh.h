/*
 * Copyright (C) 2011 Motorola Mobility, Inc.
 */

#ifndef MOTOROLA_SENSORHUB_INTERFACE_H
#define MOTOROLA_SENSORHUB_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>
#include <linux/stm401.h>

__BEGIN_DECLS

#define SENSORHUB_HARDWARE_MODULE_ID "sensorhub"

// Motorola defined sensor types
#define SENSOR_TYPE_DISPLAY_ROTATE      SENSOR_TYPE_DEVICE_PRIVATE_BASE
#define SENSOR_TYPE_FLAT_UP             SENSOR_TYPE_DEVICE_PRIVATE_BASE + 1
#define SENSOR_TYPE_FLAT_DOWN           SENSOR_TYPE_DEVICE_PRIVATE_BASE + 2
#define SENSOR_TYPE_STOWED              SENSOR_TYPE_DEVICE_PRIVATE_BASE + 3
#define SENSOR_TYPE_CAMERA_ACTIVATE     SENSOR_TYPE_DEVICE_PRIVATE_BASE + 4
#define SENSOR_TYPE_IR_GESTURE          SENSOR_TYPE_DEVICE_PRIVATE_BASE + 5
#define SENSOR_TYPE_IR_RAW              SENSOR_TYPE_DEVICE_PRIVATE_BASE + 7
#define SENSOR_TYPE_CHOPCHOP_GESTURE    SENSOR_TYPE_DEVICE_PRIVATE_BASE + 10
#define SENSOR_TYPE_LIFT_GESTURE        SENSOR_TYPE_DEVICE_PRIVATE_BASE + 11
#define SENSOR_TYPE_MOTO_GLANCE_GESTURE SENSOR_TYPE_DEVICE_PRIVATE_BASE + 12
#define SENSOR_TYPE_MOTO_MOD_CURRENT_DRAIN SENSOR_TYPE_DEVICE_PRIVATE_BASE + 13
#define SENSOR_TYPE_SENSOR_SYNC         SENSOR_TYPE_DEVICE_PRIVATE_BASE + 14
#define SENSOR_TYPE_ULTRASOUND_GESTURE  SENSOR_TYPE_DEVICE_PRIVATE_BASE + 15

// com.motorola.slpc.modalitymanager algorithms
#define SENSORHUB_ALGO_MODALITY         STM401_IDX_MODALITY
#define SENSORHUB_ALGO_ORIENTATION      STM401_IDX_ORIENTATION
#define SENSORHUB_ALGO_STOWED           STM401_IDX_STOWED
#define SENSORHUB_ALGO_ACCUM_MODALITY   STM401_IDX_ACCUM_MODALITY
#define SENSORHUB_ALGO_ACCUM_MVMT       STM401_IDX_ACCUM_MVMT

// original movement algorithm
#define SENSORHUB_ALGO_MOVEMENT         5

// number of algos supported
#define SENSORHUB_NUM_ALGOS             6

// number of accum reqs supported
#define SENSORHUB_NUM_ACCUM_REQS        4

#define SENSORHUB_EVENT_TRANSITION      0
#define SENSORHUB_EVENT_ACCUM_STATE     1
#define SENSORHUB_EVENT_ACCUM_MVMT      2

// original events
#define SENSORHUB_EVENT_START_MOVEMENT  4
#define SENSORHUB_EVENT_END_MOVEMENT    5

// generic callback event
#define SENSORHUB_EVENT_GENERIC_CB      6

//reset event
#define SENSORHUB_EVENT_RESET           7

// number of "parts" supported by algos
#define SENSORHUB_NUM_MODALITIES        6
#define SENSORHUB_NUM_ORIENTATIONS      4
#define SENSORHUB_NUM_STOWED            3

// accumulated state-time request
typedef struct {
    uint16_t states;
    uint16_t window_s;
    uint16_t threshold_s;
} sensorhub_accum_state_req_t;

// accumulated movement request
typedef struct {
    uint16_t time_s;
    uint16_t distance;
    uint16_t holdoff;
} sensorhub_accum_mvmt_req_t;

// per "part" request info
struct sensorhub_req_t {
    union {
        uint16_t durations[2];
        struct {
            uint16_t start_dur_s;
            uint16_t end_dur_s;
        };
        sensorhub_accum_state_req_t as_req;
        sensorhub_accum_mvmt_req_t am_req;
    };
    // elapsedRealtime()
    int64_t req_time_ms;
};

// original enable
struct sensorhub_algo_t {
    uint32_t type;
    uint32_t enable;
    uint32_t id;
    float parameter[2];
};

struct sensorhub_event_t {
    uint32_t type;
    int64_t time;
    int64_t ertime;
    union {
        // original events
        float value[3];
        // transition
        struct {
            uint16_t algo;
            bool past;
            uint8_t confidence;
            uint32_t old_state;
            uint32_t new_state;
        };
        // accum state
        struct {
            uint16_t accum_algo;
            uint16_t id;
        };
        // accum mvmt
        struct {
            uint32_t time_s;
            uint32_t distance;
        };
    };
};

struct sensorhub_device_t {
    struct hw_device_t common;
    int (*enable)(struct sensorhub_device_t* device, struct sensorhub_algo_t* algo);
    int (*algo_req)(struct sensorhub_device_t* device, uint16_t algo, uint32_t active_parts, struct sensorhub_req_t* req_info);
    int (*algo_query)(struct sensorhub_device_t* device, uint16_t algo, struct sensorhub_event_t* output);
    int (*poll)(struct sensorhub_device_t* device, struct sensorhub_event_t* event);
};

__END_DECLS

#endif /* MOTOROLA_SENSORHUB_INTERFACE_H */
