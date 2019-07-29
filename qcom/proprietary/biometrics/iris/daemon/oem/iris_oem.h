/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IRIS_OEM_H__
#define __IRIS_OEM_H__

#include <stdint.h>
#include <ui/Rect.h>

using namespace android;

class iris_oem_led {
public:
    enum IRIS_LED_MODE {
        IRIS_LED_MODE_FLASH,
        IRIS_LED_MODE_LED,
        IRIS_LED_MODE_MAX,
    };
public:
    virtual ~iris_oem_led() {};

    virtual int get_version(int &version) = 0;
    virtual int get_mode(int &led_mode) = 0;
    virtual int get_intensity_range(int &min, int &max, int &step) = 0;
    virtual int set_intensity(int intensity) = 0;
public:
    static const int IRIS_OEM_LED_VERSION = 1;
};


enum IRIS_CAM_TYPE {
    IRIS_CAM_TYPE_IRIS,/* iris camera */
    IRIS_CAM_TYPE_PREVIEW,/* preview camera if not same as iris camera */
    IRIS_CAM_TYPE_MAX,
};

struct iris_oem_camera_info {
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint32_t prev_width;
    uint32_t prev_height;
    Rect prev_crop;
    uint32_t prev_transform;
    uint32_t min_exposure_ms;
    uint32_t max_exposure_ms;
    uint32_t default_exposure_ms;
    uint32_t min_gain;
    uint32_t max_gain;
    uint32_t default_gain;
};

struct iris_oem_camera_config {
    uint32_t cam_num;
    struct iris_oem_camera_info cam_info[IRIS_CAM_TYPE_MAX];
};

class iris_oem_cam {
public:
    virtual ~iris_oem_cam() {};

    virtual int get_version(int &version) = 0;
    virtual int get_camera_config(struct iris_oem_camera_config &config) = 0;
public:
    static const int IRIS_OEM_CAM_VERSION = 1;
};

extern "C"  int create_iris_oem_obj(iris_oem_led **led, iris_oem_cam **cam);

#endif

