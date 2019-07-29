/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IRIS_QTI_DEFAULT_H__
#define __IRIS_QTI_DEFAULT_H__

#include "iris_oem.h"
#include <system/window.h>

class iris_qti_led : public iris_oem_led {
public:
    iris_qti_led();
    virtual ~iris_qti_led();

    virtual int get_version(int &version);
    virtual int get_mode(int &led_mode);
    virtual int get_intensity_range(int &min, int &max, int &step);
    virtual int set_intensity(int intensity);
};

class iris_qti_cam : public iris_oem_cam {
public:
    iris_qti_cam();
    virtual ~iris_qti_cam();

    virtual int get_version(int &version);
    virtual int get_camera_config(struct iris_oem_camera_config &config);

private:
    int get_rgb_camera_preview_setting(uint32_t &width, uint32_t &height, Rect &crop, uint32_t &transform);
};


#endif

