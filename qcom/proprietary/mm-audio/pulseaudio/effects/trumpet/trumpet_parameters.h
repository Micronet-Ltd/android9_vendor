/*
 * Copyright (c) 2018-2019, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _TRUMPET_PARAMETERS_H_
#define _TRUMPET_PARAMETERS_H_

#include <stdint.h>     //for uint32_t and int32_t etc

#define DAWS_PARAM_ENABLE                               0x10010904
#define DAWS_PARAM_PREGAIN                              0x10010905
#define DAWS_PARAM_POSTGAIN                             0x10010906
#define DAWS_PARAM_SYSTEMGAIN                           0x10010907
#define DAWS_PARAM_MI_DV_LEVELER_STEERING               0x10010908
#define DAWS_PARAM_MI_DIALOG_ENHANCER                   0x10010909
#define DAWS_PARAM_MI_SURROUND_COMPRESSOR               0x10010910
#define DAWS_PARAM_MI_IEQ_STEERING                      0x10010911
#define DAWS_PARAM_DIALOG_AMOUNT                        0x10010912
#define DAWS_PARAM_DIALOG_DUCKING                       0x10010913
#define DAWS_PARAM_DIALOG_ENABLE                        0x10010914
#define DAWS_PARAM_VOLUME_LEVELER_AMOUNT                0x10010915
#define DAWS_PARAM_VOLUME_LEVELER_IN_TARGET             0x10010916
#define DAWS_PARAM_VOLUME_LEVELER_OUT_TARGET            0x10010917
#define DAWS_PARAM_VOLUME_LEVELER_ENABLE                0x10010918
#define DAWS_PARAM_VOLUME_MODELER_CALIBRATION           0x10010919
#define DAWS_PARAM_VOLUME_MODELER_ENABLE                0x10010920
#define DAWS_PARAM_VOLMAX_BOOST                         0x10010921
#define DAWS_PARAM_BASS_BOOST                           0x10010922
#define DAWS_PARAM_BASS_CUTOFF_FREQ                     0x10010923
#define DAWS_PARAM_BASS_WIDTH                           0x10010924
#define DAWS_PARAM_BASS_ENABLE                          0x10010925
#define DAWS_PARAM_BASS_EXTRACT_CUTOFF_FREQ             0x10010926
#define DAWS_PARAM_BASS_EXTRACT_ENABLE                  0x10010927
#define DAWS_PARAM_REGULATOR_SET                        0x10010928
#define DAWS_PARAM_REGULATOR_OVERDRIVE                  0x10010929
#define DAWS_PARAM_REGULATOR_TIMBRE_PRESERVE            0x10010930
#define DAWS_PARAM_REGULATOR_RELAXATION_AMT             0x10010931
#define DAWS_PARAM_REGULATOR_SPKR_DIST_ENABLE           0x10010932
#define DAWS_PARAM_REGULATOR_ENABLE                     0x10010933
#define DAWS_PARAM_VIRTUAL_BASS_MODE                    0x10010934
#define DAWS_PARAM_VIRTUAL_BASS_SRC_FREQ                0x10010935
#define DAWS_PARAM_VIRTUAL_BASS_MIX_FREQ                0x10010936
#define DAWS_PARAM_VIRTUAL_BASS_OVERALL_GAIN            0x10010937
#define DAWS_PARAM_VIRTUAL_BASS_SUBGAINS                0x10010938
#define DAWS_PARAM_VIRTUAL_BASS_SLOPE_GAIN              0x10010939
#define DAWS_PARAM_FRONT_SPK_ANG                        0x10010940
#define DAWS_PARAM_SURROUND_SPK_ANG                     0x10010941
#define DAWS_PARAM_HEIGHT_SPK_ANG                       0x10010942
#define DAWS_PARAM_HEIGHT_FILTER_MODE                   0x10010943
#define DAWS_PARAM_SURROUND_BOOST                       0x10010944
#define DAWS_PARAM_SURROUND_DECODER_ENABLE              0x10010945
#define DAWS_PARAM_CALIBRATION                          0x10010946
#define DAWS_PARAM_OP_MODE                              0x10010947
#define DAWS_PARAM_GRAPHICS_ENABLE                      0x10010948
#define DAWS_PARAM_GRAPHICS_SET                         0x10010949
#define DAWS_PARAM_AUDIO_OPTIMIZER_ENABLE               0x10010950
#define DAWS_PARAM_AUDIO_OPTIMIZER_SET                  0x10010951
#define DAWS_PARAM_PROCESS_OPTIMIZER_ENABLE             0x10010952
#define DAWS_PARAM_PROCESS_OPTIMIZER_SET                0x10010953
#define DAWS_PARAM_IEQ_ENABLE                           0x10010954
#define DAWS_PARAM_IEQ_AMOUNT                           0x10010955
#define DAWS_PARAM_IEQ_SET                              0x10010956

#define DAWS_DAP_BANDS_MAX                              20
#define DAWS_DAP_CHANNEL_MAX                            12
#define DAP_MAX_MIX_MATRIX_SIZE                         100

#define DAWS_DAP_SUBGAIN_MAX                             3
/*
    To Bypass Trumpet
*/
typedef struct audproc_dap_enable_param_t {
    uint32_t enable;
} audproc_dap_enable_param_t;
/*
    * Pre-gain specifies the amount of gain which has been applied to the signal before entering the signal chain.
*/
typedef struct audproc_dap_pregain_param_t {
    int32_t pregain;
} audproc_dap_pregain_param_t;
/*
    * Post-gain specifies the amount of gain which will be applied to the signal externally after leaving the signal chain.
      For example: by an analog volume control.
*/
typedef struct audproc_dap_postgain_param_t {
    int32_t postgain;
} audproc_dap_postgain_param_t;
/*
    * System gain specifies the amount of gain which should be applied by the signal chain.
*/
typedef struct audproc_dap_systemgain_param_t {
    int32_t systemgain;
} audproc_dap_systemgain_param_t;
/*
    * Media Intelligence can be used to supply information to the Intelligent
    * Equalizer to improve the quality of processing it does.
    * The Media Intelligence feature can be used to steer the Intelligent
    * Equalizer, Volume Leveler, Dialog Enhancer and Surround Compressor.
*/
typedef struct audproc_dap_mi_dv_leveler_steering_enable_param_t {
    uint32_t mi_dv_leveler_steering_enable;
} audproc_dap_mi_dv_leveler_steering_enable_param_t;

typedef struct audproc_dap_mi_dialog_enhancer_steering_enable_param_t {
    uint32_t mi_dialog_enhancer_steering_enable;
} audproc_dap_mi_dialog_enhancer_steering_enable_param_t;

typedef struct audproc_dap_mi_surround_compressor_steering_enable_param_t {
    uint32_t mi_surround_compressor_steering_enable;
} audproc_dap_mi_surround_compressor_steering_enable_param_t;

typedef struct audproc_dap_mi_ieq_steering_enable_param_t {
    uint32_t mi_ieq_steering_enable;
} audproc_dap_mi_ieq_steering_enable_param_t;

/*
    * amount : The Dialog Enhancer amount controls the strength of the Dialog Enhancer effect.
*/
typedef struct audproc_dap_dialog_amount_param_t {
    int32_t amount;
} audproc_dap_dialog_amount_param_t;

/*
    * ducking: When dialog is detected, Dialog Enhancer also supports attenuating channels which are not the source of the dialog.
*/
typedef struct audproc_dap_dialog_ducking_param_t {
    int32_t ducking;
} audproc_dap_dialog_ducking_param_t;

/*
    * enable : Enables or disables the Dialog Enhancement feature.
*/
typedef struct audproc_dap_dialog_enable_param_t {
    int32_t enable;
} audproc_dap_dialog_enable_param_t;

/*
    * Enable         : Enables or disables the Volume Leveler
    * leveler amount : Sets the aggressiveness of the Volume Leveler.
    * volmax boost   : Sets the boost applied to the signal by the Volume Maximizer.
    * in target      : the average loudness level of the incoming audio specified according to a K loudness weighting.
    * out target     : average loudness level which the audio should be moved to.
    * calibration    : The Volume Modeler calibration parameter is used to fine-tune the
                       manufacturer calibrated reference level to the listening environment.
    * volmax boost   : Controls the amount of gain applied by the Volume Maximizer while the
                       Volume Leveler is enabled.
*/
typedef struct audproc_dap_volume_leveler_amount_param_t {
    int32_t volume_leveler_amount;
} audproc_dap_volume_leveler_amount_param_t;

typedef struct audproc_dap_volume_leveler_in_target_param_t {
    int32_t volume_leveler_in_target;
} audproc_dap_volume_leveler_in_target_param_t;

typedef struct audproc_dap_volume_leveler_out_target_param_t {
    int32_t volume_leveler_out_target;
} audproc_dap_volume_leveler_out_target_param_t;

typedef struct audproc_dap_volume_leveler_enable_param_t {
    int32_t volume_leveler_enable;
} audproc_dap_volume_leveler_enable_param_t;

typedef struct audproc_dap_volume_modeler_calibration_param_t {
    int32_t volume_modeler_calibration;
} audproc_dap_volume_modeler_calibration_param_t;

typedef struct audproc_dap_volume_modeler_enable_param_t {
    int32_t volume_modeler_enable;
} audproc_dap_volume_modeler_enable_param_t;

typedef struct audproc_dap_volmax_boost_param_t {
    int32_t volmax_boost;
} audproc_dap_volmax_boost_param_t;

/*
    * be enable   : Bass Enhancer enhances the bass response of a speaker by adding a low
                    frequency shelf.
    * be boost    : Sets the amount of bass-boost applied by Bass Enhancer.
    * be cutoff   : Sets cutoff frequency of the filter used by Bass Enhancer.
    * be width    : Sets the width of the boost curve in octaves below the cutoff
                    frequency.
    * bex enable  : Enables or disables the Bass Extraction Feature.
    * bex cutoff  : Sets the cutoff frequency (in Hz) for Bass Extraction
*/
typedef struct audproc_dap_bass_boost_param_t {
    int32_t be_boost;
} audproc_dap_bass_boost_param_t;

typedef struct audproc_dap_bass_cutoff_freq_param_t {
    int32_t be_cutoff_freq;
} audproc_dap_bass_cutoff_freq_param_t;

typedef struct audproc_dap_bass_width_param_t {
    int32_t be_width;
} audproc_dap_bass_width_param_t;

typedef struct audproc_dap_bass_enable_param_t {
    int32_t be_enable;
} audproc_dap_bass_enable_param_t;

typedef struct audproc_dap_bass_extraction_cutoff_freq_param_t {
    int32_t bex_cutoff_freq;
} audproc_dap_bass_extraction_cutoff_freq_param_t;

typedef struct audproc_dap_bass_extraction_enable_param_t {
    int32_t bex_enable;
} audproc_dap_bass_extraction_enable_param_t;

/*
    * bands num      : Number of bands of data supplied to the regulator.
    * center freq    : Center frequencies of each band.
    * low_thresholds : Lower excitation thresholds which when exceeded will
                       cause the Audio Regulator to begin attenuating.
    * high_thresholds: Upper excitation thresholds which will not be
                       permitted to be exceeded.
    * isolated_bands : An array of booleans specifying whether the band
                       is isolated from the gain-frequency smoothing.
                       Non-zero values in this array indicate that the
                       band is isolated and surrounding bands will not
                       be affected by the amount of regulation being
                       applied to it.
    * overdrive      : sets a boost which will be applied to all of the tuned low
                       and high threshol
    * timbre preserve: sets the timbre preservation amount for the Audio Regulator.
    * Relaxation amt : This function sets the audio regulator distortion relaxation amount.
    * spkr distortion: This function sets the operating mode of the Audio Regulator. (Peak protection and speaker distortion).
    * regulator      : enables or disables the Audio Regulator.
*/
typedef struct audproc_dap_regulator_set_param_t {
    int32_t bands_num;
    uint32_t freqs[DAWS_DAP_BANDS_MAX];
    int32_t thresh_low[DAWS_DAP_BANDS_MAX];
    int32_t thresh_high[DAWS_DAP_BANDS_MAX];
    int32_t isolated_bands[DAWS_DAP_BANDS_MAX];
} audproc_dap_regulator_set_param_t;

typedef struct audproc_dap_regulator_overdrive_param_t {
    int32_t overdrive;
} audproc_dap_regulator_overdrive_param_t;

typedef struct audproc_dap_regulator_timbre_preservation_param_t {
    int32_t timbre_preservation;
} audproc_dap_regulator_timbre_preservation_param_t;

typedef struct audproc_dap_regulator_relaxation_amount_param_t {
    int32_t relaxation_amount;
} audproc_dap_regulator_relaxation_amount_param_t;

typedef struct audproc_dap_regulator_spkr_distortion_enable_param_t {
    int32_t speaker_dist_enable;
} audproc_dap_regulator_spkr_distortion_enable_param_t;

typedef struct audproc_dap_regulator_enable_param_t {
    int32_t regulator_enable;
} audproc_dap_regulator_enable_param_t;

/*
    * mode         : delay | 2nd order harmonic | 2nd and 3rd order harmonic | 2nd, 3rd and 4th order harmonic
    * low src freq : lowest frequency boundaries that defines the source frequency range for Virtual Bass transpose.
    * hign src freq: highest frequency boundaries that defines the source frequency range for Virtual Bass transpose.
    * overall gain : overall gain which is applied to the output of transposer.
    * slope gain   : slope gain which is used to adjust the envelope of the transposer output.
    * sub gain     : subgains for the 2nd, 3rd and 4th harmonics.
    * low mix freq : lower frequency boundaries that limit the transposed harmonics which need to be mixed.
    * high mix freq: high frequency boundaries that limit the transposed harmonics which need to be mixed.
*/
typedef struct audproc_dap_virtual_bass_mode_param_t {
    int32_t mode;
} audproc_dap_virtual_bass_mode_param_t;

typedef struct audproc_dap_virtual_bass_src_freq_param_t {
    int32_t low_src_freq;
    int32_t high_src_freq;
} audproc_dap_virtual_bass_src_freq_param_t;

typedef struct audproc_dap_virtual_bass_overall_gain_param_t {
    int32_t overall_gain;
} audproc_dap_virtual_bass_overall_gain_param_t;

typedef struct audproc_dap_virtual_bass_slope_gain_param_t {
    int32_t slope_gain;
} audproc_dap_virtual_bass_slope_gain_param_t;

typedef struct audproc_dap_virtual_bass_subgain_param_t {
    int32_t subgains[DAWS_DAP_SUBGAIN_MAX];
} audproc_dap_virtual_bass_subgain_param_t;

typedef struct audproc_dap_virtual_bass_mix_freq_param_t {
    int32_t low_mix_freq;
    int32_t high_mix_freq;
} audproc_dap_virtual_bass_mix_freq_param_t;

/*
    * front spkr angle  : Sets the angle between the central listening plane and the listener
                          for the front speaker pair.
    * surround angle    : Sets the angle between the central listening plane and the listener
                          for the surround speaker pair.
    * height filter mode: DISABLED | FRONT FIRING | UP FIRING
    * height angle      : Sets the angle between the central listening plane and the listener
                          for the height front/middle speaker pair.
*/
typedef struct audproc_dap_front_spk_ang_param_t {
    uint32_t virtualizer_front_speaker_angle;
} audproc_dap_front_spk_ang_param_t;

typedef struct audproc_dap_surround_spk_ang_param_t {
    uint32_t virtualizer_surround_speaker_angle;
} audproc_dap_surround_spk_ang_param_t;

typedef struct audproc_dap_height_filter_mode_param_t {
    uint32_t height_filter_mode;
} audproc_dap_height_filter_mode_param_t;

typedef struct audproc_dap_height_spk_ang_param_t {
    uint32_t virtualizer_height_speaker_angle;
} audproc_dap_height_spk_ang_param_t;

/*
    * surround boost : Sets the maximum boost applied by the Surround Compressor.
    * decoder enable : enable or disable the surround decoder.
*/
typedef struct audproc_dap_surround_boost_param_t {
    uint32_t surround_boost;
} audproc_dap_surround_boost_param_t;

typedef struct audproc_dap_surround_decoder_param_t {
    uint32_t surround_decoder_enable;
} audproc_dap_surround_decoder_param_t;

/*
    * calibration boost : The Volume Modeler calibration parameter is used to fine-tune the
                          manufacturer calibrated reference level to the listening environment
*/
typedef struct audproc_dap_calibration_param_t {
    uint32_t calibration_boost;
} audproc_dap_calibration_param_t;


/*
    * enable  : Enables or disables for the Graphic Equalizer feature.
    * num     : number of bands present
    * freqs   : center frequencies for each band
    * gains   : gains for each band
*/
typedef struct audproc_dap_graphic_enable_param_t {
    int32_t graphic_equalizer_enable;
} audproc_dap_graphic_enable_param_t;

typedef struct audproc_dap_graphic_set_param_t {
    uint32_t graphic_equalizer_num;
    uint32_t graphic_equalizer_freqs[DAWS_DAP_BANDS_MAX];
    int32_t graphic_equalizer_gains[DAWS_DAP_BANDS_MAX];
} audproc_dap_graphic_set_param_t;

/*
    * enable  : Enables or disables the process optimizer
    * num     : number of bands present
    * freqs   : center frequencies for each band
    * gains   : gains for each band per channel
*/
typedef struct audproc_dap_process_optimizer_enable_param_t {
    int32_t proc_optimizer_enable;
} audproc_dap_process_optimizer_enable_param_t;

typedef struct audproc_dap_process_optimizer_set_param_t {
    int32_t proc_optimizer_gain[DAWS_DAP_CHANNEL_MAX][DAWS_DAP_BANDS_MAX];
    uint32_t proc_optimizer_freq[DAWS_DAP_BANDS_MAX];
    uint32_t proc_optimizer_bands_num;
} audproc_dap_process_optimizer_set_param_t;

/*
    * enable  : Enables or disables audio optimizer
    * num     : number of bands present
    * freqs   : center frequencies for each band
    * gains   : gains for each band per channel
*/
typedef struct audproc_dap_audio_optimizer_enable_param_t {
    int32_t audio_optimizer_enable;
} audproc_dap_audio_optimizer_enable_param_t;

typedef struct audproc_dap_audio_optimizer_set_param_t {
    uint32_t audio_optimizer_bands_num;
    int32_t audio_optimizer_gains[DAWS_DAP_CHANNEL_MAX][DAWS_DAP_BANDS_MAX];
    uint32_t audio_optimizer_freq[DAWS_DAP_BANDS_MAX];
} audproc_dap_audio_optimizer_set_param_t;

/*
    * enable    : Enables or disables the Intelligent Equalizer feature.
    * amount    : Sets the strength of the Intelligent Equalizer effect.
    * freqs     : center frequencies for each band
    * gains     : gains for each band
    * ieq num   : number of ieq bands
    * bands set : ieq profile
*/
typedef struct audproc_dap_ieq_enable_param_t {
    int32_t ieq_enable;
} audproc_dap_ieq_enable_param_t;

typedef struct audproc_dap_ieq_amount_param_t {
    int32_t ieq_amount;
} audproc_dap_ieq_amount_param_t;

typedef struct audproc_dap_ieq_param_t {
    uint32_t ieq_num;
    uint32_t ieq_freqs[DAWS_DAP_BANDS_MAX];
    int32_t ieq_gains[DAWS_DAP_BANDS_MAX];
} audproc_dap_ieq_param_t;

/*
   * spkr_virt_mode     : Virtualization mode (i.e. channel format in which to process output like 5.1.2 or 7.1)
   * size_of_matrix     : Size of mixure matrix to be used. 0 means NULL will be passed. This is required if the channel
                        : format requires different output channel count than actual output channel count
                        : e.g. output processing in 5.1.2 and then downmixing to stereo channel will require mixure matrix
   * mixer_matrix sub  : Values in matrix
   * spkr_out_mode     : processing mode
   * side_spkr_enabled : Is side speaker present or not
   * enable sub        : Is sub-woofer present or not
   * height channel    : Number of height channels
*/
typedef struct audproc_dap_op_mode_param_t {
   int32_t spkr_virt_mode;      //number of main speaker
   int32_t size_of_matrix;
   int32_t mixer_matrix[DAP_MAX_MIX_MATRIX_SIZE];
   int32_t spkr_out_mode;      //number of main speaker
   int32_t side_spkr_enabled;
   int32_t enable_sub;
   int32_t height_channels;
} audproc_dap_op_mode_param_t;


#endif // _TRUMPET_PARAMETERS_H_

