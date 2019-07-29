/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include "iris_qti_default.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cutils/properties.h>

iris_qti_led::iris_qti_led()
{
}

iris_qti_led::~iris_qti_led()
{
}

int iris_qti_led::get_version(int &version)
{
	version = IRIS_OEM_LED_VERSION;
	return 0;
}

int iris_qti_led::get_mode(int &led_mode)
{
	led_mode = IRIS_LED_MODE_FLASH;
	return 0;
}

int iris_qti_led::get_intensity_range(int &min, int &max, int &step)
{
	min = 0;
	max = 1;
	step = 1;
	return 0;
}

int iris_qti_led::set_intensity(int intensity)
{
	return -1;
}




iris_qti_cam::iris_qti_cam()
{
}

iris_qti_cam::~iris_qti_cam()
{
}


int iris_qti_cam::get_version(int &version)
{
	version = IRIS_OEM_CAM_VERSION;
	return 0;
}

int iris_qti_cam::get_camera_config(struct iris_oem_camera_config &config)
{

    char pval[PROPERTY_VALUE_MAX];
    int property_val;

    property_get("persist.iris.rgb_preview", pval, "1");
    property_val = atoi(pval);
    if (property_val) {
        config.cam_num = 2;
        config.cam_info[IRIS_CAM_TYPE_IRIS].id = 2;
        config.cam_info[IRIS_CAM_TYPE_IRIS].width = 1920;
        config.cam_info[IRIS_CAM_TYPE_IRIS].height = 1920;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_width = 640;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_height = 320;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_transform = 0;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_crop = Rect(0, 0, 0, 0);
        config.cam_info[IRIS_CAM_TYPE_IRIS].min_exposure_ms = 1;
        config.cam_info[IRIS_CAM_TYPE_IRIS].max_exposure_ms = 50;
        config.cam_info[IRIS_CAM_TYPE_IRIS].default_exposure_ms = 20;
        config.cam_info[IRIS_CAM_TYPE_IRIS].min_gain = 1;
        config.cam_info[IRIS_CAM_TYPE_IRIS].max_gain = 100;
        config.cam_info[IRIS_CAM_TYPE_IRIS].default_gain = 50;

        config.cam_info[IRIS_CAM_TYPE_PREVIEW].id = 1;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].width = 1920;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].height = 1080;
        get_rgb_camera_preview_setting(config.cam_info[IRIS_CAM_TYPE_PREVIEW].prev_width,
                config.cam_info[IRIS_CAM_TYPE_PREVIEW].prev_height,
                config.cam_info[IRIS_CAM_TYPE_PREVIEW].prev_crop,
                config.cam_info[IRIS_CAM_TYPE_PREVIEW].prev_transform);
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].min_exposure_ms = 1;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].max_exposure_ms = 50;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].default_exposure_ms = 20;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].min_gain = 1;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].max_gain = 100;
        config.cam_info[IRIS_CAM_TYPE_PREVIEW].default_gain = 50;
    } else {
        config.cam_num = 1;
        config.cam_info[IRIS_CAM_TYPE_IRIS].id = 2;
        config.cam_info[IRIS_CAM_TYPE_IRIS].width = 1920;
        config.cam_info[IRIS_CAM_TYPE_IRIS].height = 1920;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_width = 640;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_height = 320;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_transform = 0;
        config.cam_info[IRIS_CAM_TYPE_IRIS].prev_crop = Rect(0, 0, 0, 0);
        config.cam_info[IRIS_CAM_TYPE_IRIS].min_exposure_ms = 1;
        config.cam_info[IRIS_CAM_TYPE_IRIS].max_exposure_ms = 50;
        config.cam_info[IRIS_CAM_TYPE_IRIS].default_exposure_ms = 20;
        config.cam_info[IRIS_CAM_TYPE_IRIS].min_gain = 1;
        config.cam_info[IRIS_CAM_TYPE_IRIS].max_gain = 100;
        config.cam_info[IRIS_CAM_TYPE_IRIS].default_gain = 50;
    }

    return 0;
}

int iris_qti_cam::get_rgb_camera_preview_setting(uint32_t &width, uint32_t &height, Rect &crop, uint32_t &transform)
{
    uint32_t display_width = 1920, display_height = 1080, crop_x = 912, crop_y = 220, crop_width = 417, crop_height = 834;
//    uint32_t display_width = 640, display_height = 480, crop_x = 0, crop_y = 0, crop_width = 640, crop_height = 480;
    FILE *cfg_file = NULL;
    char *cfg_file_name = (char *)"/data/misc/iris/display.cfg";

    cfg_file = fopen(cfg_file_name, "r");
    if (!cfg_file) {
        ALOGE("failed to open iris display cfg file %s, use default settings \n", cfg_file_name);
    } else {
        fscanf(cfg_file, "%d %d %d %d %d %d", &display_width, &display_height, &crop_x, &crop_y, &crop_width, &crop_height);
        ALOGD("Get display settings from cfg file %d %d %d %d %d %d", display_width, display_height, crop_x, crop_y, crop_width, crop_height);
    }

    //8998 mtp settings
    width = display_width;
    height = display_height;
    crop = Rect(crop_x, crop_y, crop_x + crop_height * height / width -1, crop_y + crop_height -1);
    transform = NATIVE_WINDOW_TRANSFORM_ROT_90 | NATIVE_WINDOW_TRANSFORM_FLIP_V;
    return 0;
}

int create_iris_oem_obj(iris_oem_led **led, iris_oem_cam **cam)
{
	*led = new iris_qti_led();
	*cam = new iris_qti_cam();
	return 0;
}


