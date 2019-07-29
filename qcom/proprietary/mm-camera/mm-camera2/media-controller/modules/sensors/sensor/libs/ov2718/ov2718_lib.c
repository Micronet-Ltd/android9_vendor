/* ov2718_lib.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "ov2718_lib.h"
#include <utils/Log.h> 
#include "debug_lib.h"  

#undef SLOW
#undef SDBG
#define OV2718_DEBUG
#ifdef OV2718_DEBUG
#define SLOW(fmt, args...) ALOGE("%s:%d," fmt "\n",__func__,__LINE__, ##args)
#define SDBG(fmt, args...) SERR(fmt, ##args)
#else
#define SLOW(fmt, args...) do { } while (0)
#define SDBG(fmt, args...) do { } while (0)
 
#endif

#define AWB_GAIN_R 0x3360
#define AWB_GAIN_GR 0x3362
#define AWB_GAIN_GB 0x3364
#define AWB_GAIN_B 0x3366

#define AWB_GAIN_R_OFFSET 0x3378
#define AWB_GAIN_GR_OFFSET 0x337B
#define AWB_GAIN_GB_OFFSET 0x337E
#define AWB_GAIN_B_OFFSET 0x3381

#define BL 64
#define BLC 256



/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static unsigned int sensor_real_to_register_gain(float gain)
{
  uint16_t reg_analog_gain;
  float analog_realGain = gain/MIN_DIGITAL_GAIN;
      
  if (analog_realGain < 2) {
      gain = 1;
  } else if (analog_realGain < 4) {
      gain = 1;
  } else if (analog_realGain < 8) {
      gain = 2;
  } else {
      gain = 3;
  }
    
  gain = (gain) * 1.0;
  reg_analog_gain = (uint16_t) gain;

  return reg_analog_gain;
}

/**
 * FUNCTION: sensor_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float sensor_register_to_real_gain(unsigned int reg_gain)
{
  float real_gain = 0;

  if (reg_gain == 0) {
    real_gain = 1.0;
  } else if (reg_gain == 1) {
    real_gain = 2.0;
  } else if (reg_gain == 2) {
    real_gain = 4.0;
  } else if (reg_gain == 3) {
    real_gain = 8.0;
  } else {
    real_gain = 1.0;
  }

  return real_gain;
}

/**
 * FUNCTION: sensor_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int sensor_digital_gain_calc(float real_gain, float sensor_real_gain)
{
  unsigned int reg_dig_gain = 0;
  float real_dig_gain = 0;

  if (real_gain > MAX_ANALOG_GAIN) {
    real_dig_gain = real_gain / sensor_real_gain;
  } else {
    real_dig_gain = MIN_DIGITAL_GAIN;
  }

  if (real_dig_gain > MAX_DIGITAL_GAIN) {
    real_dig_gain = MAX_DIGITAL_GAIN;
  }

  reg_dig_gain = (unsigned int)(real_dig_gain * OV2718_MAX_DGAIN_DECIMATOR);

  return reg_dig_gain;
}


/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static int sensor_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
  if (!exp_info) {
    return -1;
  }

  exp_info->reg_gain = sensor_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain =
    sensor_register_to_real_gain(exp_info->reg_gain);
  exp_info->sensor_digital_gain =
    sensor_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
  exp_info->sensor_real_dig_gain =
    (float)exp_info->sensor_digital_gain / OV2718_MAX_DGAIN_DECIMATOR;
  exp_info->digital_gain =
    real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
  exp_info->line_count = line_count;

  return 0;
}

/**
 * FUNCTION: sensor_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exposure_array(unsigned int gain,
  __attribute__((unused))unsigned int digital_gain, unsigned int line,
  unsigned int fl_lines, __attribute__((unused)) int luma_avg,
  __attribute__((unused)) unsigned int fgain,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
  int rc = 0;
  unsigned short reg_count = 0;
  unsigned short i = 0;

  if (!reg_setting) {
    return -1;
  }

  fl_lines = ((fl_lines + 1) >> 1) << 1;

  for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xff00) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line & 0x00ff;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = ((gain & 0x03)|0x01) << 2;
  reg_count++;
  
  SLOW("fl_lines                                     = %d",fl_lines);
  SLOW("line                                         = %d",line);
  SLOW("gain                                         = %d",gain);
  SLOW("reg_setting->reg_setting[reg_count].reg_data = %d",reg_setting->reg_setting[reg_count-1].reg_data);

  for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;
  return 0;
}

/**
 * FUNCTION: sensor_fill_awb_array
 *
 * DESCRIPTION: Fill the AWB HDR array
 **/
static int sensor_fill_awb_array(unsigned short awb_gain_r,
  unsigned short awb_gain_b, struct camera_i2c_seq_reg_setting* reg_setting)
{
    unsigned short reg_count = 0;
    float isp_gain_r = 0, isp_gain_b = 0, isp_gain_g = 0;
    float r_offset = 0, b_offset = 0, g_offset = 0;

    isp_gain_r = (float)awb_gain_r/256.0;
    isp_gain_b = (float)awb_gain_b/256.0;
    isp_gain_g = 1.0;

    r_offset = (isp_gain_r - 1) * BL;
    b_offset = (isp_gain_b - 1) * BL;
    g_offset = (isp_gain_g - 1) * BL;

    SLOW("isp_gain_r = %f, isp_gain_b = %f, isp_gain_g = %f", isp_gain_r, isp_gain_b, isp_gain_g);
    SLOW("r_offset = %f, b_offset = %f, g_offset = %f", r_offset, b_offset, g_offset);

    /* Formula of Sensor AWB Gain */
    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_R;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_r * BLC) & 0xFF00) >> 8;
    SLOW("AWB_GAIN_R: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_R + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_r * BLC) & 0xFF);
    SLOW("AWB_GAIN_R: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GR;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_g * BLC) & 0xFF00) >> 8;
    SLOW("AWB_GAIN_GR: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GR + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_g * BLC) & 0xFF);
    SLOW("AWB_GAIN_GR: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GB;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_g * BLC) & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GB + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_g * BLC) & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_B;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_b * BLC) & 0xFF00) >> 8;
    SLOW("AWB_GAIN_B: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_B + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(isp_gain_b * BLC) & 0xFF);
    SLOW("AWB_GAIN_b: %d", reg_setting->reg_setting[reg_count].reg_data[0]);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    /* Formula of Sensor AWB Offset */
    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_R_OFFSET;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(r_offset * BLC) & 0xFF0000) >> 16;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_R_OFFSET + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(r_offset * BLC) & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_R_OFFSET + 2;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(r_offset * BLC) & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;


    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GR_OFFSET;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF0000) >> 16;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GR_OFFSET + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GR_OFFSET + 2;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;


    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GB_OFFSET;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF0000) >> 16;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GB_OFFSET + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_GB_OFFSET + 2;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(g_offset * BLC) & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;


    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_B_OFFSET;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(b_offset * BLC) & 0xFF0000) >> 16;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_B_OFFSET + 1;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(b_offset * BLC) & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr = AWB_GAIN_B_OFFSET + 2;
    reg_setting->reg_setting[reg_count].reg_data[0] = ((unsigned int)(b_offset * BLC) & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 1;
    reg_count++;

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
    reg_setting->delay = 0;

    return 0;
}



/**
 * FUNCTION: sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
void *sensor_open_lib(void)
{
  return &sensor_lib_ptr;
}
