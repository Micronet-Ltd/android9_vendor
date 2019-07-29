/* qtech_ov5693_eeprom.c
 *
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "qtech_ov5693_eeprom.h"

struct camera_i2c_reg_setting g_reg_setting;
struct camera_i2c_reg_array g_reg_array[AWB_REG_SIZE + LSC_REG_SIZE + 1];

/** qtech_ov5693_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void qtech_ov5693_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = FALSE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;
}

/** index: index of otp group. (1, 2, 3)
 *return: 0, group index is empty
 * 1, group index has invalid data
 * 2, group index has valid data
**/
int check_otp_wb(int index, sensor_eeprom_data_t *e_ctrl)
{
  int flag = 0;
  int addr_offset = 0;

  addr_offset = index * 16;
  flag = e_ctrl->eeprom_params.buffer[addr_offset];
  flag = flag & 0xc0;
  if (flag == 0x00)
  {
    return 0;
  }
  else if (flag & 0x80)
  {
    return 1;
  }
  else
  {
    return 2;
  }
}

/** index: index of otp group. (1, 2, 3)
 *return: 0, group index is empty
 * 1, group index has invalid data
 * 2, group index has valid data
**/
int check_otp_lenc(int index, sensor_eeprom_data_t *e_ctrl)
{
  int flag = 0;
  int addr_offset = 0;

  addr_offset = (index * 4) * 16;
  flag = e_ctrl->eeprom_params.buffer[addr_offset];
  flag = flag & 0xc0;
  if (flag == 0x00)
  {
    return 0;
  }
  else if (flag & 0x80)
  {
    return 1;
  }
  else
  {
    return 2;
  }
}


/** qtech_ov5693_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void qtech_ov5693_update_awb()
{
  uint16_t R_gain, G_gain, B_gain;
  uint16_t G_gain_R, G_gain_B ;
  uint16_t nR_G_gain, nB_G_gain, nG_G_gain;
  uint16_t nBase_gain;

  DEBUG_INFO("rg_ratio=0x%x,bg_ratio=0x%x", otp_data.rg_ratio, otp_data.bg_ratio);

  if(otp_data.bg_ratio < BG_RATIO_TYPICAL_VALUE)
  {
    if (otp_data.rg_ratio < RG_RATIO_TYPICAL_VALUE)
    {
      G_gain = 0x400;
      B_gain = 0x400 * BG_RATIO_TYPICAL_VALUE / otp_data.bg_ratio;
      R_gain = 0x400 * RG_RATIO_TYPICAL_VALUE / otp_data.rg_ratio;
    }
    else
    {
      R_gain = 0x400;
      G_gain = 0x400 * otp_data.rg_ratio / RG_RATIO_TYPICAL_VALUE;
      B_gain = G_gain * BG_RATIO_TYPICAL_VALUE / otp_data.bg_ratio;
    }
  }
  else
  {
    if (otp_data.rg_ratio < RG_RATIO_TYPICAL_VALUE)
    {
      B_gain = 0x400;
      G_gain = 0x400 * otp_data.bg_ratio / BG_RATIO_TYPICAL_VALUE;
      R_gain = G_gain * RG_RATIO_TYPICAL_VALUE / otp_data.rg_ratio;
    }
    else
    {
      G_gain_B = 0x400 * otp_data.bg_ratio / BG_RATIO_TYPICAL_VALUE;
      G_gain_R = 0x400 * otp_data.rg_ratio / RG_RATIO_TYPICAL_VALUE;
      if(G_gain_B > G_gain_R )
      {
        B_gain = 0x400;
        G_gain = G_gain_B;
        R_gain = G_gain * RG_RATIO_TYPICAL_VALUE / otp_data.rg_ratio;
      }
      else
      {
        R_gain = 0x400;
        G_gain = G_gain_R;
        B_gain = G_gain * BG_RATIO_TYPICAL_VALUE / otp_data.bg_ratio;
      }
    }
  }

  DEBUG_INFO("R_gain =0x%x,G_gain=0x%x,B_gain=0x%x",R_gain,G_gain,B_gain) ;

  if (R_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3400;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3401;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (G_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3402;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3403;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (B_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3404;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3405;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0x00ff;
    g_reg_setting.size++;
  }
}

/** qtech_ov5693_update_lenc:
 *
 * Update lense
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void qtech_ov5693_update_lenc(sensor_eeprom_data_t *e_ctrl)
{
  int i = 0;
  for(i = 0;i < LSC_REG_SIZE;i++)
  {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5800 + i;
    g_reg_array[g_reg_setting.size].reg_data = otp_data.lenc[i];
    g_reg_setting.size++;
  }
  g_reg_array[g_reg_setting.size].reg_addr = 0x5000;
  g_reg_array[g_reg_setting.size].reg_data = ((uint8_t)e_ctrl->eeprom_params.buffer[256]) | 0x80;
  g_reg_setting.size++;
}

/** qtech_ov5693_read_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read white balance success or not.
 **/
static int qtech_ov5693_read_wbdata(int group_index, sensor_eeprom_data_t *e_ctrl)
{
  uint8_t temp;
  uint8_t reg_val;
  int addr_offset = -1;
  int group_size = 16;
  SLOW("Enter");

  addr_offset = group_index * group_size;
  otp_data.module_integrator_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 1]);
  otp_data.lens_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 2]);
  otp_data.production_year =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 3]);
  otp_data.production_month =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 4]);
  otp_data.production_day =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 5]);

  temp = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 10]);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 6]);
  otp_data.rg_ratio = (reg_val << 2) + ((temp >> 6) & 0x03) ;

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 7]);
  otp_data.bg_ratio = (reg_val << 2) + ((temp >> 4) & 0x03) ;
  return 0;
}

/** index: index of otp group. (1, 2, 3)
 * otp_ptr: pointer of otp_struct
 * return: 0,
 **/
int qtech_ov5693_read_lenc(int group_index, sensor_eeprom_data_t *e_ctrl)
{
  int addr_offset = -1;
  int group_size = 16 * 4;
  int i = 0;
  SLOW("Enter");

  addr_offset = LSC_OFFSET + (group_index - 1) * group_size;
  for (i = 0; i < LSC_REG_SIZE; i++)
  {
    otp_data.lenc[i] = e_ctrl->eeprom_params.buffer[addr_offset + i + 1];
  }

  return 0;
}


/** qtech_ov5693_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void qtech_ov5693_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  SLOW("Enter");
  int rc = 0;
  struct otp_struct current_otp;
  int i;
  int otp_index;
  int temp;

   // check first lens correction OTP with valid data
  for(i = 1; i <= 3; i++)
  {
    temp = check_otp_wb(i, e_ctrl);
    if (temp == 2)
    {
      otp_index = i;
      break;
    }
  }
  if (i > 3)
  {
    // no valid LSC OTP data
    return;
  }

  rc = qtech_ov5693_read_wbdata(otp_index, e_ctrl);
  if(rc < 0){
    SERR("read wbdata failed");
    return;
  }
  qtech_ov5693_update_awb();

  SLOW("Exit");
}

/** qtech_ov5693_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: 0 success, 1 fail.
 **/
int qtech_ov5693_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  struct otp_struct current_otp;
  int i;
  int otp_index;
  int temp;
  // check first lens correction OTP with valid data
  for(i=1;i<=3;i++)
  {
    temp = check_otp_lenc(i, e_ctrl);
    if (temp == 2)
    {
      otp_index = i;
      break;
    }
  }
  if (i > 3)
  {
    // no valid LSC OTP data
    return 1;
  }
  qtech_ov5693_read_lenc(otp_index, e_ctrl);
  qtech_ov5693_update_lenc(e_ctrl);
  // success
  return 0;
}

/** qtech_ov5693_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void qtech_ov5693_format_calibration_data(void *e_ctrl) {
  SLOW("Enter");
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  qtech_ov5693_format_wbdata(ectrl);
  /* No need to update lensshading because OV deleted LSC from OTP */
  //qtech_ov5693_format_lensshading(ectrl);
  SLOW("Exit");
}

/** qtech_ov5693_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
int qtech_ov5693_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return 0;
}

/** qtech_ov5693_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* qtech_ov5693_eeprom_open_lib(void) {
  return &qtech_ov5693_lib_func_ptr;
}

