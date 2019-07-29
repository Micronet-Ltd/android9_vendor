/*
 * Copyright (c) 2012-2013, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define MAIN_C
#include <string.h>
#include <errno.h>
#include <jni.h>
#include <common_log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <display_config.h>
#include <unistd.h>
#include <vendor/qti/hardware/qdutils_disp/1.0/IQdutilsDisp.h>

#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>

#define LST_SOCKET_NAME "suilst"
#define SVC_SOCKET_NAME "suisvc"

extern "C"
{

using namespace android;
#include "com_qualcomm_qti_services_secureui_SecureUIService.h"
#include "SecureDisplayInternals.h"

using vendor::qti::hardware::qdutils_disp::V1_0::IQdutilsDisp;


static struct {
  char name[UNIX_PATH_MAX];
  size_t len;
  uint32_t used;
} remote_party = {};

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_init(JNIEnv * /*env*/, jclass /*cls*/)
{
  int rv = -1;
  const char *lst_name = LST_SOCKET_NAME;
  const char *svc_name = SVC_SOCKET_NAME;
  do {
    rv = svc_sock_init(svc_name,strlen(svc_name),lst_name,strlen(lst_name),0);
    if (rv) {
      break;
    }
  } while (0);

  return (rv);
}

extern "C" void
Java_com_qualcomm_qti_services_secureui_SecureUIService_terminate
  (JNIEnv * /*env*/, jclass /*cls*/)
{
  svc_sock_terminate();
}

extern "C" jbyteArray
Java_com_qualcomm_qti_services_secureui_SecureUIService_waitForMessage
  (JNIEnv * env, jclass /*cls*/, jbyteArray input )
{
  jbyteArray jarray = NULL;
  uint8_t mex[BYTES_IN_MEX] = {0};
  jbyte* arrayBytes = NULL;
  jbyte* arrayBytes_input = NULL;
  int rv = 0;
  secdisp_ip secdisp_input;
  do {

    remote_party.len = UNIX_PATH_MAX;
    rv = svc_sock_wait_for_message(mex,BYTES_IN_MEX, &secdisp_input,remote_party.name, &remote_party.len);
    if (rv) {
      LOGE("Error waiting for a message");
      break;
    }

    remote_party.used = 0;
    LOGD("Message source: 0%s (%u bytes)", (char*)remote_party.name+1, (unsigned int)remote_party.len);

    jarray=(env)->NewByteArray(BYTES_IN_MEX);
    if (jarray == NULL) {
      LOGE("Error creating jarray");
      break;
    }
    arrayBytes = (env)->GetByteArrayElements(jarray, NULL);
    if (arrayBytes == NULL) {
      LOGE("Error getting arrayBytes");
      break;
    }
    memcpy(arrayBytes, mex, BYTES_IN_MEX);

    arrayBytes_input = (env)->GetByteArrayElements(input, NULL);
    if (arrayBytes_input == NULL) {
      LOGE("Error getting arrayBytes");
      break;
    }
    memcpy(arrayBytes_input, &secdisp_input, BYTES_IN_INPUT);

  } while (0);
  if (arrayBytes) {
    (env)->ReleaseByteArrayElements(jarray, arrayBytes, 0);
  }

  if (arrayBytes_input) {
    (env)->ReleaseByteArrayElements(input, arrayBytes_input, 0);
  }
  return jarray;
}

extern "C" jbyteArray
Java_com_qualcomm_qti_services_secureui_SecureUIService_getSource
  (JNIEnv * env, jclass /*cls*/)
{
  jbyteArray jarray = NULL;
  jbyte* arrayBytes = NULL;
  do {

    if(remote_party.used == 1) {
      LOGE("Already consumed");
      break;
    }

    jarray=(env)->NewByteArray(remote_party.len);
    if (jarray == NULL) {
      LOGE("Error creating jarray");
      break;
    }
    arrayBytes = (env)->GetByteArrayElements(jarray, NULL);
    if (arrayBytes == NULL) {
      LOGE("Error getting arrayBytes");
      break;
    }
    memcpy(arrayBytes, remote_party.name, remote_party.len);
    LOGD("Returning source: 0%s (%u bytes)", (char*)remote_party.name+1, (unsigned int)remote_party.len);
  } while (0);
  if (arrayBytes) {
    (env)->ReleaseByteArrayElements(jarray, arrayBytes, 0);
  }
  return jarray;
}


extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_sendResponse
  (JNIEnv * env, jclass /*cls*/, jint id, jint payload, jbyteArray remote)
{
  int rv = 0;
  char name[UNIX_PATH_MAX] = {0};
  size_t len = 0;
  jbyte* arrayBytes = NULL;
  do {
    if (remote != NULL) {
      len = (env)->GetArrayLength(remote);
      if (len > UNIX_PATH_MAX) {
        LOGE("Array too long: %u vs %u max", (unsigned int)len, UNIX_PATH_MAX);
        break;
      }
      arrayBytes = (env)->GetByteArrayElements(remote, NULL);
      if (arrayBytes == NULL) {
        LOGE("Error getting arrayBytes");
        break;
      }
      memcpy(name, arrayBytes, len);
    }
    LOGD("Sending to: 0%s", (char*)name+1);
    rv = svc_sock_send_response(id, payload, name, len);
  } while (0);
  if (arrayBytes) {
    (env)->ReleaseByteArrayElements(remote, arrayBytes, 0);
  }
  return (rv);
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_sendNotification
  (JNIEnv * env, jclass /*cls*/, jint id, jint payload, jbyteArray remote)
{
  int rv = 0;
  char name[UNIX_PATH_MAX] = {0};
  size_t len = 0;
  jbyte* arrayBytes = NULL;
  do {
    if (remote != NULL) {
      len = (env)->GetArrayLength(remote);
      if (len > UNIX_PATH_MAX) {
        LOGE("Array too long: %u vs %u max", (unsigned int) len, UNIX_PATH_MAX);
        break;
      }

      arrayBytes = (env)->GetByteArrayElements(remote, NULL);
      if (arrayBytes == NULL) {
        LOGE("Error getting arrayBytes");
        break;
      }
      memcpy(name, arrayBytes, len);
    }
    rv = svc_sock_send_notification(id, payload, name, len);
  } while(0);
  if (arrayBytes) {
    (env)->ReleaseByteArrayElements(remote, arrayBytes, 0);
  }
  return (rv);
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_pauseExternal
  (JNIEnv * /*env*/, jclass /*cls*/, jint display, jint doPause)
{
  IQdutilsDisp::input data;
  sp<IQdutilsDisp> client = IQdutilsDisp::getService();
  if(client == NULL)
    return -1;
  int rv = 0;
  if (display == 1) {
    ALOGD("pauseExternal: HDMI: %d", doPause);
  } else if (display == 2) {
    ALOGD("pauseExternal: WFD: %d", doPause);
  } else {
    ALOGE("Unsupported external display: %d", display);
    return -1;
  }
  data.doPause = doPause;
  data.display = display;

  client->setSecondaryDisplayStatus(data, [&] (IQdutilsDisp::output result) {
	rv = result.status;}
  );
  if (doPause) {
    usleep(200000); // sleep 200ms
  }
  ALOGD("setSecondaryDisplayStatus: %d", rv);
  return rv;
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_externalIsConnected
  (JNIEnv *, jclass)
{
  int ret;
  sp<IQdutilsDisp> client = IQdutilsDisp::getService();
  if(client == NULL)
    return -1;
 
  client->isExternalConnected([&] (IQdutilsDisp::output result) {
		ret = result.status;}
	);
  if (ret > 0) {
    ALOGD("External connected");
    return 1;
  }
  if (ret == 0) {
    ALOGD("External not connected");
    return 0;
  }
  return -1;
}
}
