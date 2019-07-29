/* qtech_ov5693_eeprom.h
 *
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __QTECH_OV5693_EEPROM_H__
#define __QTECH_OV5693_EEPROM_H__

#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#undef DEBUG_INFO
/* #define QTECH_OV5693_DEBUG */
#ifdef QTECH_OV5693_DEBUG
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif

#define BASE_ADDR 0x3D00
#define WB_FLAG_ADDR 0x3D00

#define AWB_REG_SIZE 6
#define LSC_REG_SIZE 62

#define LSC_OFFSET 64

#define QVALUE             512
#define RG_RATIO_TYPICAL_VALUE 0x146
#define BG_RATIO_TYPICAL_VALUE 0x129

#define ABS(x)            (((x) < 0) ? -(x) : (x))

struct otp_struct {
  int module_integrator_id;
  int lens_id;
  int production_year;
  int production_month;
  int production_day;
  int rg_ratio;
  int bg_ratio;
  int lenc[62];
} otp_data;

void qtech_ov5693_get_calibration_items(void *e_ctrl);
void qtech_ov5693_format_calibration_data(void *e_ctrl);
int qtech_ov5693_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t qtech_ov5693_lib_func_ptr = {
  .get_calibration_items = qtech_ov5693_get_calibration_items,
  .format_calibration_data = qtech_ov5693_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .get_raw_data = qtech_ov5693_get_raw_data,
  .get_ois_raw_data = NULL,
  .eeprom_info =
  {
    .power_setting_array =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
      },
      .size = 10,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 8,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_STANDARD,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0x6c,
          .mem_settings =
          {
            { 0x0100, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc0, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc1, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc2, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc3, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc4, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc5, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc6, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc7, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc8, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc9, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xca, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xcb, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xcc, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xcd, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xce, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xcf, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 5 },
            { 0x3d00, CAMERA_I2C_WORD_ADDR,
              16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x5000, CAMERA_I2C_WORD_ADDR,
              1, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
          },
          .memory_map_size = 50,
        },
      },
      .size_map_array = 1,
    },
  },
};

#endif
