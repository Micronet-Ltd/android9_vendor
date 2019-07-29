/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "bt_btif_avrcp_audio_track"

#include "btif_avrcp_audio_track.h"

#include <base/logging.h>
#include <media/AudioTrack.h>
#include <media/AudioPolicyHelper.h>
#include <utils/StrongPointer.h>

#include "bt_target.h"
#include "osi/include/log.h"
#include "stack/include/a2dp_constants.h"

using namespace android;

typedef struct { android::sp<android::AudioTrack> track; } BtifAvrcpAudioTrack;

#if (DUMP_PCM_DATA == TRUE)
FILE* outputPcmSampleFile;
char outputFilename[50] = "/data/misc/bluedroid/output_sample.pcm";
#endif

#define COMPRESSED_AUDIO_BUFFER_SIZE 2048

void* BtifAvrcpAudioTrackCreate(int trackFreq, int channelType, int codec_type) {
  LOG_DEBUG(LOG_TAG, "%s Track.cpp: btCreateTrack freq %d  channel %d codec: %d",
              __func__, trackFreq, channelType, codec_type);
  audio_format_t media_format = (audio_format_t)0;
  sp<android::AudioTrack> track = NULL;

  if (codec_type == A2DP_MEDIA_CT_SBC) {
      track = new android::AudioTrack(
      AUDIO_STREAM_MUSIC, trackFreq, AUDIO_FORMAT_PCM_16_BIT, channelType,
      (size_t)0 /*frameCount*/, (audio_output_flags_t)AUDIO_OUTPUT_FLAG_DEEP_BUFFER,
      NULL /*callback_t*/, NULL /*void* user*/, 0 /*notificationFrames*/,
      AUDIO_SESSION_ALLOCATE, android::AudioTrack::TRANSFER_SYNC);
  } else if (codec_type == A2DP_MEDIA_CT_AAC || codec_type == A2DP_MEDIA_CT_NON_A2DP) {
    media_format = (audio_format_t)
            (codec_type == A2DP_MEDIA_CT_AAC ? AUDIO_FORMAT_AAC_LATM_LC : AUDIO_FORMAT_APTX);
    audio_attributes_t mAttributes;
    stream_type_to_audio_attributes(AUDIO_STREAM_MUSIC, &mAttributes);

    // code to intialize offload info
    audio_offload_info_t offload_info;
    memset(&offload_info, 0, sizeof(audio_offload_info_t));
    offload_info.size = sizeof(audio_offload_info_t);
    offload_info.sample_rate = trackFreq;
    offload_info.channel_mask = channelType;
    offload_info.format = media_format;
    offload_info.stream_type = AUDIO_STREAM_MUSIC;
    /* Note: offload_info.has_video and offload_info.is_streaming is set to true in order
     * to get offload buffer size changed to 2k in MM Audio code. This is a WAR. */
    offload_info.has_video = TRUE;
    offload_info.is_streaming = TRUE;
    offload_info.offload_buffer_size = COMPRESSED_AUDIO_BUFFER_SIZE;

    track = new android::AudioTrack(
      AUDIO_STREAM_MUSIC, trackFreq, media_format, channelType,
      (size_t)0 /*frameCount*/, (audio_output_flags_t)AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD,
      NULL /*callback_t*/, NULL /*void* user*/, 0 /*notificationFrames*/,
      AUDIO_SESSION_ALLOCATE, android::AudioTrack::TRANSFER_SYNC, &offload_info,
      -1, -1, &mAttributes);
  }
  CHECK(track != NULL);

  BtifAvrcpAudioTrack* trackHolder = new BtifAvrcpAudioTrack;
  CHECK(trackHolder != NULL);
  trackHolder->track = track;

  if (trackHolder->track->initCheck() != 0) {
    return nullptr;
  }

#if (DUMP_PCM_DATA == TRUE)
  outputPcmSampleFile = fopen(outputFilename, "ab");
#endif
  trackHolder->track->setVolume(1, 1);
  return (void*)trackHolder;
}

int BtifAvrcpAudioTrackLatency(void* handle) {
  if (handle == NULL) {
    LOG_ERROR(LOG_TAG, "%s: handle is null!", __func__);
    return 0;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  CHECK(trackHolder != NULL);
  CHECK(trackHolder->track != NULL);
  LOG_VERBOSE(LOG_TAG, "%s Track.cpp: get latency", __func__);
  return trackHolder->track->latency();
}

void BtifAvrcpAudioTrackStart(void* handle) {
  if (handle == NULL) {
    LOG_ERROR(LOG_TAG, "%s: handle is null!", __func__);
    return;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  CHECK(trackHolder != NULL);
  CHECK(trackHolder->track != NULL);
  LOG_DEBUG(LOG_TAG, "%s Track.cpp: btStartTrack", __func__);
  trackHolder->track->start();
}

void BtifAvrcpAudioTrackStop(void* handle) {
  if (handle == NULL) {
    LOG_DEBUG(LOG_TAG, "%s handle is null.", __func__);
    return;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  if (trackHolder != NULL && trackHolder->track != NULL) {
    LOG_DEBUG(LOG_TAG, "%s Track.cpp: btStopTrack", __func__);
    trackHolder->track->stop();
  }
}

void BtifAvrcpAudioTrackDelete(void* handle) {
  if (handle == NULL) {
    LOG_DEBUG(LOG_TAG, "%s handle is null.", __func__);
    return;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  if (trackHolder != NULL && trackHolder->track != NULL) {
    LOG_DEBUG(LOG_TAG, "%s Track.cpp: btDeleteTrack", __func__);
    delete trackHolder;
  }

#if (DUMP_PCM_DATA == TRUE)
  if (outputPcmSampleFile) {
    fclose(outputPcmSampleFile);
  }
  outputPcmSampleFile = NULL;
#endif
}

void BtifAvrcpAudioTrackPause(void* handle) {
  if (handle == NULL) {
    LOG_DEBUG(LOG_TAG, "%s handle is null.", __func__);
    return;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  if (trackHolder != NULL && trackHolder->track != NULL) {
    LOG_DEBUG(LOG_TAG, "%s Track.cpp: btPauseTrack", __func__);
    trackHolder->track->pause();
    trackHolder->track->flush();
  }
}

void BtifAvrcpSetAudioTrackGain(void* handle, float gain) {
  if (handle == NULL) {
    LOG_DEBUG(LOG_TAG, "%s handle is null.", __func__);
    return;
  }
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  if (trackHolder != NULL && trackHolder->track != NULL) {
    LOG_DEBUG(LOG_TAG, "%s set gain %f", __func__, gain);
    trackHolder->track->setVolume(gain);
  }
}

int BtifAvrcpAudioTrackWriteData(void* handle, void* audioBuffer,
                                 int bufferlen) {
  BtifAvrcpAudioTrack* trackHolder = static_cast<BtifAvrcpAudioTrack*>(handle);
  CHECK(trackHolder != NULL);
  CHECK(trackHolder->track != NULL);
  int retval = -1;
#if (DUMP_PCM_DATA == TRUE)
  if (outputPcmSampleFile) {
    fwrite((audioBuffer), 1, (size_t)bufferlen, outputPcmSampleFile);
  }
#endif
  retval = trackHolder->track->write(audioBuffer, (size_t)bufferlen);
  LOG_VERBOSE(LOG_TAG, "%s Track.cpp: btWriteData len = %d ret = %d", __func__,
              bufferlen, retval);
  return retval;
}
