/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "daws_util.h"
#include "trumpet_parameters.h"

#include "pa_qti_effect.h"
#include "pa_qti_effect_trumpet.h"

#ifdef TEST_LOG
#define print_log ALOGD
#define print_err ALOGE
#else
#define print_log printf
#define print_err printf
#endif

#define DAP_CPDP_PROCESS_5_1_2             (6)
#define DAP_CPDP_PROCESS_5_1_2_SPEAKER     (11)

#define MAX_INPUT_LEN                      (100)

#define QFACTOR                            (16)

static void pa_qti_set_all_param_from_xml(pa_qti_effect_handle_t effect_handle,
                                          daws_dap_config *xml_config,
                                          int32_t preset);

static void pa_qti_set_trumpet_systemgain(pa_qti_effect_handle_t effect_handle,
                                          int32_t systemgain);

static void pa_qti_set_trumpet_postgain(pa_qti_effect_handle_t effect_handle,
                                        int32_t postgain);

static void pa_qti_set_trumpet_pregain(pa_qti_effect_handle_t effect_handle,
                                       int32_t pregain);

static void pa_qti_enable_trumpet(pa_qti_effect_handle_t effect_handle,
                                  int32_t enable);

typedef struct thread_data_t {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t cond2;
    uint32_t exit;
    pthread_t thread_id;
    daws_dap_config xml_config;
    int32_t xml_config_status;
    pa_qti_effect_handle_t effect_handle;
    char user_cmd[MAX_INPUT_LEN];
} thread_data_t;

/**
******************************************************************************
* @function static void pa_qti_set_all_param_from_xml(
*                                         pa_qti_effect_handle_t effect_handle,
*                                         daws_dap_config *xml_config,
*                                         int32_t preset)
*
* @brief It calls all aDSP trumpet parameters which are available in xml file.
*
* @param effect_handle  Handle for trumpet effect
* @param xml_config     Stores all data parsed from xml file
* @param preset         dynamic | music | movie | game | voice | off
*
* @return  void
******************************************************************************
*/
static void pa_qti_set_all_param_from_xml(pa_qti_effect_handle_t effect_handle,
                                        daws_dap_config *xml_config,
                                        int32_t preset)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int32_t);
    /* This size can be used for all parameters with single integer value */
    uint32_t array_size = sizeof(pa_qti_effect_param_t) \
                               + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);

    /* pregain */
    *(int32_t *)values->data = TRUMPET_PARAM_PREGAIN;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.pregain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for pregain returns %d\n", rc);

    /* postgain */
    *(int32_t *)values->data = TRUMPET_PARAM_POSTGAIN;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.postgain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for postgain returns %d\n", rc);

    /* systemgain */
    *(int32_t *)values->data = TRUMPET_PARAM_SYSTEMGAIN;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.systemgain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for sysgain returns %d\n", rc);

    /* mi dv leveler steering enable */
    *(int32_t *)values->data = TRUMPET_PARAM_MI_DV_LEVELER_STEERING;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.mi_dv_leveler_steering_enable,
          values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for mi dv %d returns %d\n",
          xml_config->profiles[preset].cp.mi_dv_leveler_steering_enable, rc);

    /* mi dialog enhancer */
    *(int32_t *)values->data = TRUMPET_PARAM_MI_DIALOG_ENHANCER;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.mi_dialog_enhancer_steering_enable,
          values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for mi de %d returns %d\n",
            xml_config->profiles[preset].cp.mi_dialog_enhancer_steering_enable,
            rc);

    /* mi surround compressor */
    *(int32_t *)values->data = TRUMPET_PARAM_MI_SURROUND_COMPRESSOR;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.mi_surround_compressor_steering_enable,
        values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for mi compressor %d returns %d\n",
        xml_config->profiles[preset].cp.mi_surround_compressor_steering_enable,
        rc);

    /* mi ieq steering */
    *(int32_t *)values->data = TRUMPET_PARAM_MI_IEQ_STEERING;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.mi_ieq_steering_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for mi ieq %d returns %d\n",
                xml_config->profiles[preset].cp.mi_ieq_steering_enable, rc);

    /* dialog enhancer amount */
    *(int32_t *)values->data = TRUMPET_PARAM_DIALOG_AMOUNT;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.dialog_enhancer_amount, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for dialog amount %d returns %d\n",
              xml_config->profiles[preset].cp.dialog_enhancer_amount, rc);

    /* dialog ducking */
    *(int32_t *)values->data = TRUMPET_PARAM_DIALOG_DUCKING;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.dialog_enhancer_ducking, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for dialog ducking %d returns %d\n",
          xml_config->profiles[preset].cp.dialog_enhancer_ducking, rc);

    /* dialog enable */
    *(int32_t *)values->data = TRUMPET_PARAM_DIALOG_ENABLE;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.dialog_enhancer_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for dialog enable %d returns %d\n",
          xml_config->profiles[preset].cp.dialog_enhancer_enable, rc);

    /* volume leveler amount */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_LEVELER_AMOUNT;
    memcpy((values->data + values->psize),
         &xml_config->profiles[preset].cp.volume_leveler_amount, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol leveler amt %d returns %d\n",
              xml_config->profiles[preset].cp.volume_leveler_amount, rc);

    /* volume leveler in target */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_LEVELER_IN_TARGET;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.volume_leveler_in_target, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol lev in target %d returns %d\n",
              xml_config->profiles[preset].cp.volume_leveler_in_target, rc);

    /* volume leveler out target */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_LEVELER_OUT_TARGET;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.volume_leveler_out_target,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol leveler \
                   out target %d returns %d\n",
          xml_config->profiles[preset].cp.volume_leveler_out_target, rc);

    /* volume leveler enable */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_LEVELER_ENABLE;
    memcpy((values->data + values->psize),
         &xml_config->profiles[preset].cp.volume_leveler_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol leveler enable %d \
                  returns %d\n",
                  xml_config->profiles[preset].cp.volume_leveler_enable, rc);

    /* volume modeler calibration */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_MODELER_CALIBRATION;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.volume_modeler_calibration,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol calibration %d returns %d\n",
            xml_config->profiles[preset].cp.volume_modeler_calibration, rc);

    /* volume modeler enable */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLUME_MODELER_ENABLE;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.volume_modeler_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vol modeler enable %d \
          returns %d\n",
          xml_config->profiles[preset].cp.volume_modeler_enable, rc);

    /* volmax boost */
    *(int32_t *)values->data = TRUMPET_PARAM_VOLMAX_BOOST;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.volmax_boost, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for volmax boost %d returns %d\n",
              xml_config->profiles[preset].cp.volmax_boost, rc);

    /* bass boost */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_BOOST;
    memcpy((values->data + values->psize),
           &xml_config->profiles[preset].cp.be_boost, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for bass boost %d returns %d\n",
              xml_config->profiles[preset].cp.be_boost, rc);

    /* cut off freq bass */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_CUTOFF_FREQ;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.be_cutoff_freq, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for cut off freq bass %d returns %d\n",
            xml_config->profiles[preset].cp.be_cutoff_freq, rc);

    /* bass width */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_WIDTH;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.be_width, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for bass width %d returns %d\n",
          xml_config->profiles[preset].cp.be_width, rc);

    /* bass enable */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_ENABLE;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.be_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for bass enable %d returns %d\n",
              xml_config->profiles[preset].cp.be_enable, rc);

    /* cut off freq bass extraction */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_EXTRACT_CUTOFF_FREQ;
    memcpy((values->data + values->psize),
          &xml_config->profiles[preset].cp.bex_cutoff_freq, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for cut off freq bass \
              extraction %d returns %d\n",
              xml_config->profiles[preset].cp.bex_cutoff_freq, rc);

    /* bass extraction enable */
    *(int32_t *)values->data = TRUMPET_PARAM_BASS_EXTRACT_ENABLE;
    memcpy((values->data + values->psize),
            &xml_config->profiles[preset].cp.bex_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for bass extraction \
              enable %d returns %d\n",
              xml_config->profiles[preset].cp.bex_enable, rc);

    /* front speaker angle */
    *(int32_t *)values->data = TRUMPET_PARAM_FRONT_SPK_ANG;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.virtualizer_front_speaker_angle,
        values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for front spkr angle %d returns %d\n",
          xml_config->profiles[preset].cp.virtualizer_front_speaker_angle, rc);

    /* surround speaker angle */
    *(int32_t *)values->data = TRUMPET_PARAM_SURROUND_SPK_ANG;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.virtualizer_surround_speaker_angle,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for surround spkr angle %d \
          returns %d\n",
          xml_config->profiles[preset].cp.virtualizer_surround_speaker_angle,
          rc);

    /* height speaker angle */
    *(int32_t *)values->data = TRUMPET_PARAM_HEIGHT_SPK_ANG;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.virtualizer_height_speaker_angle,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for height speaker angle %d \
          returns %d\n",
          xml_config->profiles[preset].cp.virtualizer_height_speaker_angle, rc);

    /* height filter mode */
    *(int32_t *)values->data = TRUMPET_PARAM_HEIGHT_FILTER_MODE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.height_filter_mode, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for height filter mode %d \
          returns %d\n",
          xml_config->profiles[preset].cp.height_filter_mode, rc);

    /* surround boost */
    *(int32_t *)values->data = TRUMPET_PARAM_SURROUND_BOOST;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.surround_boost, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for surround boost %d returns %d\n",
            xml_config->profiles[preset].cp.surround_boost, rc);

    /* surround decoder enable */
    *(int32_t *)values->data = TRUMPET_PARAM_SURROUND_DECODER_ENABLE;
    memcpy((values->data + values->psize),
        &xml_config->profiles[preset].cp.surround_decoder_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for surround decoder enable %d returns\
          %d\n",
          xml_config->profiles[preset].cp.surround_decoder_enable, rc);

    /* calibration boost */
    *(int32_t *)values->data = TRUMPET_PARAM_CALIBRATION;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.calibration_boost, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for calibration boost %d returns %d\n",
          xml_config->profiles[preset].cp.calibration_boost, rc);

    /* regulator overdrive */
    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_OVERDRIVE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.regulator.overdrive, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for regulator overdrive %d \
          returns %d\n",
          xml_config->profiles[preset].cp.regulator.overdrive, rc);

    /* timbre preservation */
    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_TIMBRE_PRESERVE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.regulator.timbre_preservation,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for timbre preservation %d \
            returns %d\n",
            xml_config->profiles[preset].cp.regulator.timbre_preservation, rc);

    /* relaxation_amount */
    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_RELAXATION_AMT;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.regulator.relaxation_amount,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for relaxation_amount %d returns %d\n",
          xml_config->profiles[preset].cp.regulator.relaxation_amount, rc);

    /* speaker distortion enable */
    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_SPKR_DIST_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.regulator.speaker_dist_enable,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for speaker distortion enable %d \
          returns %d\n",
          xml_config->profiles[preset].cp.regulator.speaker_dist_enable, rc);

    /* regulator enable */
    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.regulator_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for regulator enable %d returns %d\n",
          xml_config->profiles[preset].cp.regulator_enable, rc);

    /* graphics equalizer enable */
    *(int32_t *)values->data = TRUMPET_PARAM_GRAPHICS_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.graphic_equalizer_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for graphics equalizer enable %d \
          returns %d\n",
          xml_config->profiles[preset].cp.graphic_equalizer_enable, rc);

    /* audio optimizer enable */
    *(int32_t *)values->data = TRUMPET_PARAM_AUDIO_OPTIMIZER_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.audio_optimizer_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for audio optimizer enable %d \
          returns %d\n",
          xml_config->profiles[preset].cp.audio_optimizer_enable, rc);

    /* process optimizer enable */
    *(int32_t *)values->data = TRUMPET_PARAM_PROCESS_OPTIMIZER_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.proc_optimizer_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for process optimizer enable %d \
          returns %d\n",
          xml_config->profiles[preset].cp.proc_optimizer_enable, rc);

    /* ieq enable */
    *(int32_t *)values->data = TRUMPET_PARAM_IEQ_ENABLE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.ieq_enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for ieq enable %d returns %d\n",
          xml_config->profiles[preset].cp.ieq_enable, rc);

    /* ieq amount */
    *(int32_t *)values->data = TRUMPET_PARAM_IEQ_AMOUNT;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.ieq_amount, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for ieq amount %d returns %d\n",
          xml_config->profiles[preset].cp.ieq_amount, rc);

    /* vb mode */
    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_MODE;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.vb.mode,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb mode %d returns %d\n",
          xml_config->profiles[preset].cp.vb.mode, rc);

    /* vb overall_gain */
    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_OVERALL_GAIN;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.vb.overall_gain,
      values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb overall_gain %d returns %d\n",
          xml_config->profiles[preset].cp.vb.overall_gain, rc);

    /* vb slope_gain */
    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_SLOPE_GAIN;
    memcpy((values->data + values->psize),
      &xml_config->profiles[preset].cp.vb.slope_gain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb slope_gain %d returns %d\n",
          xml_config->profiles[preset].cp.vb.slope_gain, rc);

    /*
     * From this point, all the parameters have different size.
     * Till this point, each parameter was 4 bytes long.
     */

    /* regulator tuning */
    audproc_dap_regulator_set_param_t regulator;

    /* This size can be used for all parameters with single integer value */
    array_size = sizeof(pa_qti_effect_param_t) + sizeof(int32_t) \
               + sizeof(audproc_dap_regulator_set_param_t);
    uint32_t buf_regulator[array_size];

    values = (pa_qti_effect_param_t *)buf_regulator;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_regulator_set_param_t);

    regulator.bands_num   = xml_config->profiles[preset].cp.regulator.bands_num;
    memcpy(regulator.freqs, xml_config->profiles[preset].cp.regulator.freqs,
          sizeof(regulator.freqs));
    memcpy(regulator.thresh_low,
          xml_config->profiles[preset].cp.regulator.thresh_low,
          sizeof(regulator.thresh_low));
    memcpy(regulator.thresh_high,
          xml_config->profiles[preset].cp.regulator.thresh_high,
          sizeof(regulator.thresh_high));
    memcpy(regulator.isolated_bands,
          xml_config->profiles[preset].cp.regulator.isolated_bands,
          sizeof(regulator.isolated_bands));

    *(int32_t *)values->data = TRUMPET_PARAM_REGULATOR_SET;
    memcpy((values->data + values->psize), &regulator, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for regulator tuning returns %d\n",
            rc);

    /* graphics equalizer set */
    audproc_dap_graphic_set_param_t graph;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_graphic_set_param_t);
    uint32_t buf_graph[array_size];

    values = (pa_qti_effect_param_t *)buf_graph;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_graphic_set_param_t);

    graph.graphic_equalizer_num
                     = xml_config->profiles[preset].cp.graphic_equalizer_num;
    memcpy(graph.graphic_equalizer_freqs,
          xml_config->profiles[preset].cp.graphic_equalizer_freqs,
          sizeof(graph.graphic_equalizer_freqs));
    memcpy(graph.graphic_equalizer_gains,
          xml_config->profiles[preset].cp.graphic_equalizer_gains,
          sizeof(graph.graphic_equalizer_gains));

    *(int32_t *)values->data = TRUMPET_PARAM_GRAPHICS_SET;
    memcpy((values->data + values->psize), &graph, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for GEQ set returns %d\n", rc);

    /* audio optimizer set */
    audproc_dap_audio_optimizer_set_param_t aud;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_audio_optimizer_set_param_t);
    uint32_t buf_aud[array_size];

    values = (pa_qti_effect_param_t *)buf_aud;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_audio_optimizer_set_param_t);

    aud.audio_optimizer_bands_num
                  = xml_config->profiles[preset].cp.audio_optimizer_bands_num;
    memcpy(aud.audio_optimizer_freq,
          xml_config->profiles[preset].cp.audio_optimizer_freq,
          sizeof(aud.audio_optimizer_freq));
    memcpy(aud.audio_optimizer_gains,
          xml_config->profiles[preset].cp.audio_optimizer_gains,
          sizeof(aud.audio_optimizer_gains));

    *(int32_t *)values->data = TRUMPET_PARAM_AUDIO_OPTIMIZER_SET;
    memcpy((values->data + values->psize), &aud, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for audio optimizer set returns %d\n",
                rc);

    /* process optimizer set */
    audproc_dap_process_optimizer_set_param_t proc;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_process_optimizer_set_param_t);
    uint32_t buf_proc[array_size];

    values = (pa_qti_effect_param_t *)buf_proc;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_process_optimizer_set_param_t);

    proc.proc_optimizer_bands_num
                = xml_config->profiles[preset].cp.proc_optimizer_bands_num;
    memcpy(proc.proc_optimizer_freq,
          xml_config->profiles[preset].cp.proc_optimizer_freq,
          sizeof(proc.proc_optimizer_freq));
    memcpy(proc.proc_optimizer_gain,
          xml_config->profiles[preset].cp.proc_optimizer_gain,
          sizeof(proc.proc_optimizer_gain));

    *(int32_t *)values->data = TRUMPET_PARAM_PROCESS_OPTIMIZER_SET;
    memcpy((values->data + values->psize), &proc, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for process optimizer set \
          returns %d\n", rc);

    /* ieq set */
    audproc_dap_ieq_param_t ieq;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t) + sizeof(audproc_dap_ieq_param_t);
    uint32_t buf_ieq[array_size];

    values = (pa_qti_effect_param_t *)buf_ieq;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_ieq_param_t);

    ieq.ieq_num         = xml_config->profiles[preset].cp.ieq_num;
    memcpy(ieq.ieq_freqs,
            xml_config->profiles[preset].cp.ieq_freqs, sizeof(ieq.ieq_freqs));
    memcpy(ieq.ieq_gains,
            xml_config->profiles[preset].cp.ieq_gains, sizeof(ieq.ieq_gains));

    *(int32_t *)values->data = TRUMPET_PARAM_IEQ_SET;
    memcpy((values->data + values->psize), &ieq, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for ieq set set returns %d\n", rc);

    /* vb_src_freq */
    audproc_dap_virtual_bass_src_freq_param_t vb_src_freq;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_virtual_bass_src_freq_param_t);
    uint32_t buf_vb_scr[array_size];

    values = (pa_qti_effect_param_t *)buf_vb_scr;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_virtual_bass_src_freq_param_t);

    vb_src_freq.low_src_freq  = xml_config->profiles[preset].cp.vb.low_src_freq;
    vb_src_freq.high_src_freq = \
                    xml_config->profiles[preset].cp.vb.high_src_freq;

    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_SRC_FREQ;
    memcpy((values->data + values->psize), &vb_src_freq, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb_src_freq set returns %d\n",
                rc);

    /* vb_mix_freq */
    audproc_dap_virtual_bass_mix_freq_param_t vb_mix_freq;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_virtual_bass_mix_freq_param_t);
    uint32_t buf_vb_mix[array_size];

    values = (pa_qti_effect_param_t *)buf_vb_mix;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_virtual_bass_mix_freq_param_t);

    vb_mix_freq.low_mix_freq  = xml_config->profiles[preset].cp.vb.low_mix_freq;
    vb_mix_freq.high_mix_freq = \
                        xml_config->profiles[preset].cp.vb.high_mix_freq;

    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_MIX_FREQ;
    memcpy((values->data + values->psize), &vb_mix_freq, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb_mix_freq set returns %d\n",
                rc);

    /* vb_subgain */
    audproc_dap_virtual_bass_subgain_param_t vb_subgain;

    array_size = sizeof(pa_qti_effect_param_t)
               + sizeof(int32_t)
               + sizeof(audproc_dap_virtual_bass_subgain_param_t);
    uint32_t buf_subgain[array_size];

    values = (pa_qti_effect_param_t *)buf_subgain;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(audproc_dap_virtual_bass_subgain_param_t);

    vb_subgain.subgains[0]     = xml_config->profiles[preset].cp.vb.subgains[0];
    vb_subgain.subgains[1]     = xml_config->profiles[preset].cp.vb.subgains[1];
    vb_subgain.subgains[2]     = xml_config->profiles[preset].cp.vb.subgains[2];

    *(int32_t *)values->data = TRUMPET_PARAM_VIRTUAL_BASS_SUBGAINS;
    memcpy((values->data + values->psize), &vb_subgain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                               array_size, (void *)values,
                               (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_err("PA QTI trumpet effect for vb_subgain set returns %d\n",
                rc);
}

/**
******************************************************************************
* @function static void pa_qti_set_trumpet_systemgain(
*                                          pa_qti_effect_handle_t effect_handle,
*                                          int32_t systemgain)
*
* @brief This function sets system gain
*
* @param effect_handle    effect handle
* @param systemgain       amount of gain
*
* @return  void
******************************************************************************
*/
static void pa_qti_set_trumpet_systemgain(pa_qti_effect_handle_t effect_handle,
                                   int32_t systemgain)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t)
                                + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);
    *(int32_t *)values->data = TRUMPET_PARAM_SYSTEMGAIN;
    memcpy((values->data + values->psize), &systemgain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                              array_size, (void *)values,
                              (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_log("%s effect systemgain failed\n", __func__);
}

/**
******************************************************************************
* @function static void pa_qti_set_trumpet_postgain(
*                                       pa_qti_effect_handle_t effect_handle,
*                                       int32_t postgain)
*
* @brief This function sets post gain
*
* @param effect_handle    effect handle
* @param systemgain       amount of gain
*
* @return  void
******************************************************************************
*/
static void pa_qti_set_trumpet_postgain(pa_qti_effect_handle_t effect_handle,
                                int32_t postgain)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t)
                               + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);
    *(int32_t *)values->data = TRUMPET_PARAM_POSTGAIN;
    memcpy((values->data + values->psize), &postgain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                              array_size, (void *)values,
                              (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_log("%s effect postgain failed\n", __func__);
}

/**
******************************************************************************
* @function static void pa_qti_set_trumpet_pregain(
*                                         pa_qti_effect_handle_t effect_handle,
*                                         int32_t pregain)
*
* @brief This function sets pre gain
*
* @param effect_handle    effect handle
* @param systemgain       amount of gain
*
* @return  void
******************************************************************************
*/
static void pa_qti_set_trumpet_pregain(pa_qti_effect_handle_t effect_handle,
                                int32_t pregain)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t)
                                        + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);
    *(int32_t *)values->data = TRUMPET_PARAM_PREGAIN;
    memcpy((values->data + values->psize), &pregain, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                              array_size, (void *)values,
                              (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_log("%s effect pregain failed\n", __func__);
}

/**
******************************************************************************
* @function static void pa_qti_enable_trumpet(
*                                     pa_qti_effect_handle_t effect_handle,
*                                     int32_t enable)
*
* @brief This function enables or disables the trumpet library
*
* @param effect_handle    effect handle
* @param enable           enable/disable (0 or 1)
*
* @return  void
******************************************************************************
*/
static void pa_qti_enable_trumpet(pa_qti_effect_handle_t effect_handle,
                           int32_t enable)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t)
                                + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);
    *(int32_t *)values->data = TRUMPET_PARAM_ENABLE;
    memcpy((values->data + values->psize), &enable, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                              array_size, (void *)values,
                              (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_log("%s effect enable/disable failed\n", __func__);
}

/**
******************************************************************************
* @function static void pa_qti_set_trumpet_virtualization(
*                                     pa_qti_effect_handle_t effect_handle,
*                                     int32_t enable)
*
* @brief This function enables or disables the Virtualization
*
* @param effect_handle    effect handle
* @param enable           enable/disable (0 or 1)
*
* @return  void
******************************************************************************
*/
static void pa_qti_set_trumpet_virtualization(
                                          pa_qti_effect_handle_t effect_handle,
                                          int32_t enable)
{
    int32_t rc = 0;
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t)
                                + sizeof(int32_t)
                                + sizeof(audproc_dap_op_mode_param_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values;
    audproc_dap_op_mode_param_t op_mode;

    memset(&op_mode, 0, sizeof(op_mode));

    if (enable == 0)
        op_mode.spkr_virt_mode = DAP_CPDP_PROCESS_5_1_2;
    else
        op_mode.spkr_virt_mode = DAP_CPDP_PROCESS_5_1_2_SPEAKER;

    op_mode.spkr_out_mode = 2;  /* default for stereo speaker */

    values = (pa_qti_effect_param_t *)buf32;
    values->psize = sizeof(int32_t);
    values->vsize = sizeof(int32_t);
    *(int32_t *)values->data = TRUMPET_PARAM_OP_MODE;
    memcpy((values->data + values->psize), &op_mode, values->vsize);
    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                              array_size, (void *)values,
                              (uint32_t *)&reply_size, (void *)&reply_data);

    if (rc != 0)
        print_log("%s effect enable/disable failed\n", __func__);
}

/**
******************************************************************************
* @function int pa_qti_parse_dlb_xml(daws_dap_config *xml_config)
*
* @brief parse xml file to obtain parameter values for different modes.
*
* @param xml_config           Structure to store parsed data
*
* @return  void
******************************************************************************
*/
int32_t pa_qti_parse_dlb_xml(daws_dap_config *xml_config)
{
    void *handle_trumpet = dlopen("/usr/lib/libtrumpetxmlparser.so", RTLD_LAZY);
    const char* error_trumpet = NULL;
    int status = -1;

    if (handle_trumpet == NULL) {
        print_err("Failed to open libtrumpetxmlparser.so with error %s\n", \
            dlerror());
    } else {
        /* prototype of the certification function */
        typedef int32_t (*_daws_dap_xml_parser)\
        (const char *, int, unsigned, void*);

        _daws_dap_xml_parser daws_dap_xml_parser = \
            (_daws_dap_xml_parser) dlsym(handle_trumpet, "daws_dap_xml_parser");
        error_trumpet = dlerror();

        if (error_trumpet != NULL)
            print_err("%s\n", error_trumpet);

        if (daws_dap_xml_parser == NULL) {
            print_err("Failed to find the function daws_dap_xml_parser \
                      in libtrumpetxmlparser.so\n");
        } else {
            status = daws_dap_xml_parser("/data/tune.xml",
                                        0,
                                        48000,
                                        xml_config);
            print_err("XML parser is success \n");
        }
        dlclose(handle_trumpet);
    }

    return status;
}

/**
******************************************************************************
* @function void* pa_qti_trumpet_effect_menu(void *data)
*
* @brief This function is used by pthread library for menu handling
*
* @param data           data to thread function
*
* @return  void*
******************************************************************************
*/
void* pa_qti_trumpet_effect_menu(void *data)
{
    thread_data_t *thr_ctxt = (thread_data_t *) data;
    const int INF = 999999999;
    int32_t pregain = INF;
    int32_t postgain = INF;
    int32_t systemgain = INF;
    int32_t preset = PRESET_DYNAMIC_MODE;
    int32_t dap_enable = 0;
    int32_t virtualization = 1;
    int32_t rc = 0;
    char user_cmd_arg[MAX_INPUT_LEN];

    user_cmd_arg[0] = '\0'; /* string with no value */

    pthread_mutex_lock(&thr_ctxt->mutex);

    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->cond, &thr_ctxt->mutex);

        if (!strncmp(thr_ctxt->user_cmd, "enable", strlen("enable"))) {
            dap_enable = 1;
            pa_qti_enable_trumpet(thr_ctxt->effect_handle, dap_enable);
        } else if (!strncmp(thr_ctxt->user_cmd, "disable", strlen("disable"))) {
            dap_enable = 0;
            pa_qti_enable_trumpet(thr_ctxt->effect_handle, dap_enable);
        } else if (!strncmp(thr_ctxt->user_cmd, "pregain", strlen("pregain"))) {
            scanf("%s", user_cmd_arg);
            pregain = atoi(user_cmd_arg);
            pa_qti_set_trumpet_pregain(thr_ctxt->effect_handle,
                                      pregain * QFACTOR);
        } else if (!strncmp(thr_ctxt->user_cmd,
                            "postgain",
                            strlen("postgain"))) {
            scanf("%s", user_cmd_arg);
            postgain = atoi(user_cmd_arg);
            pa_qti_set_trumpet_postgain(thr_ctxt->effect_handle,
                                        postgain * QFACTOR);
        } else if (!strncmp(thr_ctxt->user_cmd,
                              "systemgain",
                              strlen("systemgain"))) {
            scanf("%s", user_cmd_arg);
            systemgain = atoi(user_cmd_arg);
            pa_qti_set_trumpet_systemgain(thr_ctxt->effect_handle,
                              systemgain * QFACTOR);
        } else if (!strncmp(thr_ctxt->user_cmd,
                            "virtualization",
                            strlen("virtualization"))) {
            scanf("%s", user_cmd_arg);

            if (!strncmp(user_cmd_arg,
                        "enable",
                        strlen("enable"))) {
                virtualization = 1;
            } else if (!strncmp(user_cmd_arg, "disable", strlen("disable"))) {
                virtualization = 0;
            }
            pa_qti_set_trumpet_virtualization(thr_ctxt->effect_handle,
                                              virtualization);
        } else if (!strncmp(thr_ctxt->user_cmd, "preset", strlen("preset"))) {
            scanf("%s", user_cmd_arg);

            if (!strcmp(user_cmd_arg, "dynamic")) {
                preset = PRESET_DYNAMIC_MODE;
            } else if (!strcmp(user_cmd_arg, "movie")) {
                preset = PRESET_MOVIE_MODE;
            } else if (!strcmp(user_cmd_arg, "music")) {
                preset = PRESET_MUSIC_MODE;
            } else if (!strcmp(user_cmd_arg, "game")) {
                preset = PRESET_GAME_MODE;
            } else if (!strcmp(user_cmd_arg, "voice")) {
                preset = PRESET_VOICE_MODE;
            } else if (!strcmp(user_cmd_arg, "off")) {
                preset = PRESET_OFF_MODE;
            } else {
                preset = INF;
                print_log("INVALID PROFILE\n");
            }

           /*
            * Currently, the file tune.xml is searched in /data and is parsed.
            * If parsing is successful, then the preset is applied.
            * 2nd and 3rd arguments are not required for our purpose
            */

            if (preset != INF) {

                if (thr_ctxt->xml_config_status == 0) {
                    print_err("XML parsing successful %d\n",
                              thr_ctxt->xml_config_status);
                    pa_qti_set_all_param_from_xml(thr_ctxt->effect_handle,
                                                       &thr_ctxt->xml_config,
                                                       preset);
                } else {
                    print_err("XML parsing failed, cannot set preset %d\n",
                          thr_ctxt->xml_config_status);
                }
            }
        } else if (!strncmp(thr_ctxt->user_cmd, "exit", strlen("exit"))) {
            rc = pa_qti_port_effect_release(thr_ctxt->effect_handle);
            thr_ctxt->exit = true;

            if (rc != 0) {
                print_log("%s effect release failed\n", __func__);
                exit(4);
            }

            pthread_cond_signal(&thr_ctxt->cond2); /*signal menu thread to run*/
            break;  /* from the menu */
        } else {
            print_log("Invalid command, please try again\n");
        }

        pthread_cond_signal(&thr_ctxt->cond2); /* signal menu thread to run */
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
}

/**
******************************************************************************
* @function int main(int argc, char *argv[])
*
* @brief This application mimics the functionality of the trumpet testapp
*        provided by Dolby. It takes same parameters and sends the vaues to
*        aDSP. Any separate player can be used to handle playback.
*
* @param argc           number of command line arguments
* @param argv           arguments
*
* @return  void
******************************************************************************
*/

int main(int argc, char *argv[])
{
    int32_t rc = 0;
    thread_data_t thread_data;

    memset(&thread_data, 0, sizeof(thread_data));
    thread_data.cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    thread_data.mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    thread_data.exit = false;
    thread_data.effect_handle = NULL;

    rc = pa_qti_effect_init();

    if (rc != 0) {
        print_log("%s effect init failed with value %d\n", __func__, rc);
        exit(1);
    } else {
        print_log("pa_qti_effect_init is successful.\n");
    }

    thread_data.effect_handle = pa_qti_port_effect_create(SL_IID_TRUMPET_UUID,
                                                         "speaker");

    if (NULL == thread_data.effect_handle) {
        print_log("%s effect creation failed\n", __func__);
        exit(2);
    } else {
        print_log("pa_qti_port_effect_create : Effect created \n");
    }

    thread_data.xml_config_status = \
                    pa_qti_parse_dlb_xml (&thread_data.xml_config);

    if (pthread_create(&thread_data.thread_id,
                      NULL,
                      pa_qti_trumpet_effect_menu,
                      &thread_data)) {
        print_log("Error creating trumpet effect thread\n");
        exit(6);
    }

    while (!thread_data.exit) {

        print_log("\n******** TRUMPET EFFECT MENU ******** \n\n");
        print_log("Please type the command and press enter \n");
        print_log("enable\n");
        print_log("disable\n");
        print_log("pregain <value -130 to 30>\n");
        print_log("postgain <value -130 to 30>\n");
        print_log("systemgain <value -130 to 30>\n");
        print_log("virtualization <enable|disable>\n");
        print_log("preset <dynamic|movie|music|game|voice|off>\n");
        print_log("exit\n");

        scanf("%s", thread_data.user_cmd);

        pthread_cond_signal(&thread_data.cond); /*signal effect thread to run */

        /* sleep till effect thread runing */

        pthread_cond_wait(&thread_data.cond2, &thread_data.mutex);

        /*
         * When exit is encountered in effect thread and
         * context switching happens before release,
         * we need to wait for it to release handle.
         */

        if (thread_data.exit == true)
            pthread_join(thread_data.thread_id, NULL);
    }

    pa_qti_effect_deinit(); /* De-init the effect */

    return rc;
}
