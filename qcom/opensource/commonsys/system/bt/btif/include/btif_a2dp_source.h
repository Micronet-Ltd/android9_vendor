/******************************************************************************
 *  Copyright (C) 2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 *  NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *  GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 *  HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2016 The Android Open Source Project
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef BTIF_A2DP_SOURCE_H
#define BTIF_A2DP_SOURCE_H

#include <stdbool.h>

#include "bta_av_api.h"

typedef struct {
  bool vs_configs_exchanged;
  bool tx_started;
  bool tx_stop_initiated;
  bool tx_start_initiated;
  bool multi_vsc_support;
  bool start_reset;
} tBTIF_A2DP_SOURCE_VSC;

// Initialize and startup the A2DP Source module.
// This function should be called by the BTIF state machine prior to using the
// module.
bool btif_a2dp_source_startup(void);

// Shutdown and cleanup the A2DP Source module.
// This function should be called by the BTIF state machine during
// graceful shutdown and cleanup.
void btif_a2dp_source_shutdown(void);

// Check whether the A2DP Source media task is running.
// Returns true if the A2DP Source media task is running, otherwise false.
bool btif_a2dp_source_media_task_is_running(void);

// Check whether the A2DP Source media task is shutting down.
// Returns true if the A2DP Source media task is shutting down.
bool btif_a2dp_source_media_task_is_shutting_down(void);

// Return true if the A2DP Source module is streaming.
bool btif_a2dp_source_is_streaming(void);

// Return true if the A2DP remote is trying to start the session
bool btif_a2dp_source_is_remote_start(void);

// Cancel remote start alarm
void btif_a2dp_source_cancel_remote_start();

//return last remote started index
int btif_a2dp_source_last_remote_start_index();

void btif_a2dp_source_cancel_unblock_audio_start(void);

// Setup the A2DP Source codec, and prepare the encoder.
// This function should be called prior to starting A2DP streaming.
bt_status_t btif_a2dp_source_setup_codec(tBTA_AV_HNDL handle);

// Process a request to start the A2DP audio encoding task.
void btif_a2dp_source_start_audio_req(void);

// Process a request to stop the A2DP audio encoding task.
void btif_a2dp_source_stop_audio_req(void);

// Process a request to update the A2DP audio encoder with user preferred
// codec configuration.
// |codec_user_config| contains the preferred codec user configuration.
void btif_a2dp_source_encoder_user_config_update_req(
    const btav_a2dp_codec_config_t& codec_user_config, const RawAddress& bd_addr);

// Process a request to update the A2DP audio encoding with new audio
// configuration feeding parameters stored in |codec_audio_config|.
// The fields that are used are: |codec_audio_config.sample_rate|,
// |codec_audio_config.bits_per_sample| and |codec_audio_config.channel_mode|.
void btif_a2dp_source_feeding_update_req(
    const btav_a2dp_codec_config_t& codec_audio_config);

// Process 'idle' request from the BTIF state machine during initialization.
void btif_a2dp_source_on_idle(void);

// Process 'stop' request from the BTIF state machine to stop A2DP streaming.
// |p_av_suspend| is the data associated with the request - see
// |tBTA_AV_SUSPEND|.
void btif_a2dp_source_on_stopped(tBTA_AV_SUSPEND* p_av_suspend);

// Process 'suspend' request from the BTIF state machine to suspend A2DP
// streaming.
// |p_av_suspend| is the data associated with the request - see
// |tBTA_AV_SUSPEND|.
void btif_a2dp_source_on_suspended(tBTA_AV_SUSPEND* p_av_suspend);

// Enable/disable discarding of transmitted frames.
// If |enable| is true, the discarding is enabled, otherwise is disabled.
void btif_a2dp_source_set_tx_flush(bool enable);

// Get the next A2DP buffer to send.
// Returns the next A2DP buffer to send if available, otherwise NULL.
BT_HDR* btif_a2dp_source_audio_readbuf(void);

// Dump debug-related information for the A2DP Source module.
// |fd| is the file descriptor to use for writing the ASCII formatted
// information.
void btif_a2dp_source_debug_dump(int fd);

// Update the A2DP Source related metrics.
// This function should be called before collecting the metrics.
void btif_a2dp_source_update_metrics(void);

// Honor remote avdtp start
// This function will start a 3 second timer. If the a2dp streaming is
// started within this time, then the timer will be cancelled. Else-If
// timer expires, avdpt suspend will be issued to the remote
void btif_a2dp_source_on_remote_start(struct alarm_t *remote_start_alarm, int index);

void btif_trigger_unblock_audio_start_recovery_timer(void);
#endif /* BTIF_A2DP_SOURCE_H */
