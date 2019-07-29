/**
 * Copyright (c) 2012-2013, 2015, 2017, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOGV(...) do {} while (0)  //Comment this line to enable LOGV
#define LOGD(...) do {} while (0)  //Comment this line to enable LOGD
#include <jni.h>
#include <utils/RefBase.h>
#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>
#include <DisplayInfo.h>
#include <PixelFormat.h>

#include "gralloc_priv.h"
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <linux/fb.h>
#include "common_log.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlTransportSupport.h>

#include <tuple>
#include <vector>

#define MAX_STR_LEN (108)

extern "C"
{

class SecDisp *g_pSd = NULL;

using namespace android;

#include "com_qualcomm_qti_services_secureui_SecureUIService.h"
#include "SecureDisplayInternals.h"

struct allocated_buffer_properties{
	uintptr_t handle;
#ifndef TARGET_ION_ABI_VERSION
	struct ion_handle_data ref_ion_handle;
#endif
	android::hardware::hidl_handle native_handle;
};

static uint16_t nextBufferToPass = 0;
static std::vector<allocated_buffer_properties> allocatedBuffers;
static int32_t g_ion_dev_fd = -1;
static uint32_t u32Height_global = 0;
static uint32_t u32Width_global = 0;

SecDisp::SecDisp():numAllocatedBufs(0)
{
    mSurfaceComposer = new SurfaceComposerClient();
    if (mSurfaceComposer->initCheck() != (status_t)OK) {
       LOGE("mSurfaceComposer->initCheck() failed");
    }

    /* get display dimensions */
    mDisplayWidth = u32Width_global;
    mDisplayHeight = u32Height_global;
    LOGD("Display W x H = %d X %d\n",mDisplayWidth, mDisplayHeight);
    mControl = mSurfaceComposer->createSurface(String8("secure-ui"), mDisplayWidth,
                                               mDisplayHeight, PIXEL_FORMAT_OPAQUE);

   SurfaceComposerClient::Transaction t;
   t.setLayer(mControl, INT_MAX - 1);
   if (t.apply(true  /* synchronous */) != OK) {
        LOGE("setLayer failed");
        // TODO error handling
     }
    mSurface = mControl->getSurface();
    mNativeWindow = mSurface;

   int err = native_window_api_connect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
   if (err) {
       LOGE("native_window_api_connect failed %d", err);
   }
   LOGD("SecDisp created\n");
}


int32_t SecDisp::setConfig(const Config& config)
{
    mConfig.width       = config.width;
    mConfig.height      = config.height;
    mConfig.colorFormat = config.colorFormat;
    mConfig.usageFlags  = config.usageFlags;
    mConfig.x           = config.x;
    mConfig.y           = config.y;

    return SUCCESS;
}


int32_t SecDisp::allocateNativeBuffers (uint32_t buffersForHeap)
{
    const uint32_t numRequestedBufs = 2 + buffersForHeap; /* 2 buffers to guarantee triple-buffering */
    status_t err;

    err = native_window_set_scaling_mode(mNativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if (err != OK) {
        LOGE("native_window_set_scaling_mode failed");
        return FAILURE;
    }

    SurfaceComposerClient::Transaction t;
    t.setPosition(mControl, mConfig.x, mConfig.y);
    t.setSize(mControl, mConfig.width, mConfig.height);
    t.setFlags(mControl, layer_state_t::eLayerOpaque, layer_state_t::eLayerOpaque);
    err = t.apply(true /* synchronous */);
    if (err != OK) {

        LOGE("native_window_set_buffers_geometry failed: %d", err);
        return FAILURE;
    }

    err = native_window_set_buffers_format(mNativeWindow.get(), mConfig.colorFormat);
    if (err != OK) {
        LOGE("native_window_set_buffers_format failed: %d", err);
        return FAILURE;
    }
    err = native_window_set_buffers_dimensions(mNativeWindow.get(), mConfig.width, mConfig.height);
    if (err != OK) {
        LOGE("native_window_set_buffers_dimentions failed: %d", err);
        return FAILURE;
    }

    // Make sure to check whether video decoder
    // requested protected buffers.
    if (mConfig.usageFlags & GRALLOC1_PRODUCER_USAGE_PROTECTED ) {
        // Verify that the ANativeWindow sends images directly to
        // SurfaceFlinger.

        int queuesToNativeWindow = 0;
        err = mNativeWindow->query(
                mNativeWindow.get(), NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER,
                &queuesToNativeWindow);
        if (err != 0) {
            LOGE("error authenticating native window: %d", err);
            return FAILURE;
        }
        if (queuesToNativeWindow != 1) {
            LOGE("native window could not be authenticated");
            return FAILURE;
        }

    }

    err = native_window_set_usage(
            mNativeWindow.get(), mConfig.usageFlags);
    if (err != 0) {
        LOGE("native_window_set_usage failed: %s (%d)", strerror(-err), -err);
        return FAILURE;
    }

    int minUndequeuedBufs = 0;
    err = mNativeWindow->query(mNativeWindow.get(),
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
    if (err != 0) {
        LOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query failed: %s (%d)",
                strerror(-err), -err);
        return FAILURE;
    }
    LOGD("minUndequeuedBufs=%d",minUndequeuedBufs);

    numAllocatedBufs = numRequestedBufs + minUndequeuedBufs;

    err = native_window_set_buffer_count(
            mNativeWindow.get(), numAllocatedBufs);
    if (err != 0) {
        LOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err),
                -err);
        return FAILURE;
    }
    LOGD("Requested %d buffers",numAllocatedBufs);

    return SUCCESS;
}


int32_t SecDisp::dequeue (ANativeWindowBuffer*& dispBuf)
{
    ANativeWindowBuffer* buf;

    int err = native_window_dequeue_buffer_and_wait(mNativeWindow.get(), &buf);
    if (err != 0) {
        LOGE("dequeueBuffer failed w/ error 0x%08x", err);
        return FAILURE;
    }

    dispBuf = buf;
    return SUCCESS;
}

int32_t SecDisp::enqueue (ANativeWindowBuffer* dispBuf, bool render)
{
    ANativeWindowBuffer* buf;
    buf = dispBuf;

    int err;
    if (render) {
        LOGD("Displaying buffer [%p]",buf);
        err = mNativeWindow->queueBuffer(mNativeWindow.get(), buf, -1);
        if (err != 0) {
            LOGE("queueBuffer failed w/ error 0x%08x", err);
            return FAILURE;
        }
    } else {
        LOGD("Dropping buffer [%p]",buf);
        err = mNativeWindow->cancelBuffer(mNativeWindow.get(), buf, -1);
        if (err != 0) {
            LOGE("cancelBuffer failed w/ error 0x%08x", err);
            return FAILURE;
        }
    }

    return SUCCESS;
}


SecDisp::~SecDisp()
{
    // allow the pipes to be removed before the buffers are freed
    if ((mSurfaceComposer.get() != NULL)  && (mControl.get() != NULL)) {
        SurfaceComposerClient::Transaction t;
        t.hide(mControl);
        auto err = t.apply(true /* synchronous */);
        if (err != OK) {
            LOGE("%s: SurfaceComposerClient transaction failed (error %d)", __func__, err);
        }
    }
    native_window_api_disconnect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);

    if (mSurface.get() != NULL) {
        mSurface.clear();
        mSurface = NULL;
    }
    if (mNativeWindow.get() != NULL) {
        mNativeWindow = NULL;
    }
    if (mControl.get() != NULL) {
        mControl.clear();
        mControl = NULL;
    }
    if (mSurfaceComposer.get() != NULL) {
        mSurfaceComposer->dispose();
        mSurfaceComposer.clear();
        mSurfaceComposer = NULL;
    }
}

int32_t sec_ui_get_disp_properties(uint32_t u32Height, uint32_t u32Width)
{
    u32Height_global = u32Height;

    u32Width_global = u32Width;

    if (NULL == g_pSd)
    {
        g_pSd = new SecDisp();
        if (NULL == g_pSd)
            return FAILURE;
    }

    return SUCCESS;
}

int32_t sec_ui_start_disp(bool bDisableOverlay, uint32_t colorFormat, uint32_t u32Height, uint32_t u32Width, uint32_t u32HPos, uint32_t u32VPos, uint16_t * u16NumBuffers)
{
    if (NULL == g_pSd)
    {
        g_pSd = new SecDisp();
        if (NULL == g_pSd) {
            return FAILURE;
	}
    }

    if ((u32Height + u32VPos > g_pSd->mDisplayHeight) || (u32Width + u32HPos) > g_pSd->mDisplayWidth ){
        LOGE("sec_ui_start_disp bad parameters: u32Height = %d, u32VPos = %d, g_pSd->mDisplayHeight = %d, "
                "u32Width =%d, u32HPos = %d, g_pSd->mDisplayWidth = %d",u32Height, u32VPos, g_pSd->mDisplayHeight, u32Width, u32HPos, g_pSd->mDisplayWidth);
        return FAILURE;
    }
    if (!u16NumBuffers) {
      LOGE("NULL parameter");
      return FAILURE;
    }

    SecDisp::Config config;
    // Usage flags for securing the buffer
    config.usageFlags = GRALLOC1_PRODUCER_USAGE_PROTECTED;
    config.usageFlags |= GRALLOC1_PRODUCER_USAGE_PRIVATE_UNCACHED;
    config.usageFlags |= GRALLOC_USAGE_PRIVATE_SECURE_DISPLAY;

    if (bDisableOverlay)
    {
        if ((u32Height != g_pSd->mDisplayHeight) ||
            (u32Width != g_pSd->mDisplayWidth) ||
            (u32VPos != 0) ||
            (u32HPos != 0)) {
            LOGE("%s: Overlay disable only supported in full screen mode",__func__);

            return FAILURE;
        }
    }

    config.height = u32Height;
    config.width = u32Width;
    config.x = u32HPos;
    config.y = u32VPos;

    config.colorFormat = colorFormat;

    if (g_pSd->setConfig(config) != 0)
        return FAILURE;

    if (g_pSd->allocateNativeBuffers(*u16NumBuffers) != 0)
        return FAILURE;

    if(g_pSd->numAllocatedBufs == 0){
        return FAILURE;
    }
    *u16NumBuffers = g_pSd->numAllocatedBufs;

    return SUCCESS;
}

int32_t sec_ui_dequeue(uintptr_t *bufHandle, uint32_t *ionFd, uint32_t *u32Stride, uint32_t *u32PixelWidth)
{

    if ((NULL == bufHandle) || (NULL == ionFd) || (NULL == u32Stride) || (NULL == u32PixelWidth))
    {
        LOGE("%s: NULL pointer input",__func__);
        return FAILURE;
    }

    if (NULL == g_pSd)
    {
        LOGE("%s: Secure display wasn't initialized",__func__);
        return FAILURE;
    }

    ANativeWindowBuffer* dispBuf;
    if (g_pSd->dequeue(dispBuf) != 0) {

        return FAILURE;
   }

    *bufHandle = (uintptr_t)dispBuf;
    *ionFd = (uint32_t)(dispBuf->handle->data[0]);
    *u32Stride = dispBuf->stride;
    *u32PixelWidth = (uint32_t)bytesPerPixel(g_pSd->mConfig.colorFormat);

    return SUCCESS;
}

int32_t sec_ui_enqueue(uintptr_t bufHandle, uint8_t bToRender)
{
    if (NULL == g_pSd)
    {
        LOGE("%s: Secure display wasn't initialized",__func__);
        return FAILURE;
    }

    return (g_pSd->enqueue((ANativeWindowBuffer*)bufHandle, bToRender));
}

int32_t sec_ui_stop_disp()
{
    if (NULL == g_pSd)
    {
        LOGE("%s: Secure display wasn't initialized",__func__);
        return FAILURE;
    }

    delete(g_pSd);
    g_pSd = NULL;
    return SUCCESS;
}

#ifndef TARGET_ION_ABI_VERSION
static int32_t secui_ION_increment_ref_count(int ion_buf_fd, struct ion_handle_data *ref_handle)
{
	struct ion_fd_data ifd;
	int32_t ret = 0;

	do {
		if (!ref_handle) {
			ret = -1;
			break;
		}
		ref_handle->handle = 0;
		if (g_ion_dev_fd < 0) {
			LOGE("Error ION device not opened.");
			ret = -1;
			break;
		}
		ifd.fd = ion_buf_fd;

		if ((ret = ioctl(g_ion_dev_fd, ION_IOC_IMPORT, &ifd))) {
			LOGE("Could not increment reference count of ION buffer error = %d , ionfd = %d, g_ion_dev_fd = %d err = %d desc = %s",ret,ion_buf_fd,g_ion_dev_fd,errno,strerror(errno));
			ret = -1;
			break;
		}
		if (!ifd.handle) {
			LOGE("Reference handle NULL.");
			ret = -1;
			break;
		}
		ref_handle->handle = ifd.handle;
	} while (0);

	return ret;
}

static int32_t secui_ION_decrement_ref_count(struct ion_handle_data *ref_handle)
{
	int32_t ret;

	if (!ref_handle) {
		return -1;
	}
	if (!ref_handle->handle) {
		LOGE("Zero ION reference handle.");
		return -1;
	}
	if (g_ion_dev_fd < 0) {
		LOGE("Error ION device not opened.");
		return -1;
	}
	ret = ioctl(g_ion_dev_fd, ION_IOC_FREE, ref_handle);
	if (ret){
		LOGE("ION Memory FREE ioctl failed with ret = %d", ret);
	}
	ref_handle->handle = 0;
	return ret;
}
#endif

class JavaArrayDeleter {
public:
    JavaArrayDeleter(JNIEnv * env, jbyteArray array): env_(env), array_(array) {}
    void operator()(jbyte* ptr) {
        if (ptr) env_->ReleaseByteArrayElements(array_, ptr, 0);
    }
private:
    JNIEnv * env_;
    jbyteArray array_;
};

using JavaByteArrayPtr = std::unique_ptr<jbyte, JavaArrayDeleter>;

static std::tuple<int, std::string, secdisp_ip> unpackJavaParams(JNIEnv * env, jbyteArray input,
                                                                 jbyteArray remote) {
    std::tuple<int, std::string, secdisp_ip> result;
    int & ret = std::get<0>(result);
    std::string & remote_name = std::get<1>(result);
    secdisp_ip & input_struct = std::get<2>(result);
    ret = -1;

    if (!remote) {
        LOGE("no remote name given");
        return result;
    }
    if (!input) {
        LOGE("no input given");
        return result;
    }

    {
        auto len = (env)->GetArrayLength(remote);
        if (!len) {
            LOGE("remote name lenght cannot be 0");
            return result;
        }

        auto bytes = JavaByteArrayPtr((env)->GetByteArrayElements(remote, 0), {env, remote});
        if (!bytes) {
            LOGE("failed to get remote name bytes");
            return result;
        }

        remote_name.assign(bytes.get(), bytes.get() + len);
    }

    {
        auto len = (env)->GetArrayLength(input);
        if (len != sizeof(secdisp_ip)) {
            LOGE("input bytearray has invalid size %d (expected %zu)", len, sizeof(secdisp_ip));
            return result;
        }
        auto bytes = JavaByteArrayPtr((env)->GetByteArrayElements(input, 0), {env, input});
        if (!bytes) {
            LOGE("failed to get input bytearray");
            return result;
        }
        memcpy(&input_struct, bytes.get(), sizeof(secdisp_ip));
    }

    ret = 0;
    return result;
}


extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_getdispprop  (JNIEnv * env, jclass /*cls*/, jint id, jint payload, jbyteArray input, jbyteArray remote)
{
    int ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) return ret;

    op.status = sec_ui_get_disp_properties(ip.u32Height, ip.u32Width);
    if(op.status) return op.status;

    return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_startdisp
  (JNIEnv * env, jclass /*cls*/,jint id, jint payload, jbyteArray input, jbyteArray remote)
{
    int ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) return ret;

    g_ion_dev_fd = open("/dev/ion", O_RDONLY);
    if (g_ion_dev_fd < 0) {
        return -1;
    }

    op.u16NumBuffers = 0;
    op.status = sec_ui_start_disp((bool) ip.bDisableOverlay, (uint32_t) ip.colorFormat,
                                  (uint32_t) ip.u32Height, (uint32_t) ip.u32Width,
                                  (uint32_t) ip.u32HPos, (uint32_t) ip.u32VPos, &op.u16NumBuffers);
    if(op.status) {
        return op.status;
    }
    nextBufferToPass = 0;
    allocatedBuffers.resize(op.u16NumBuffers);
    return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                                         remote_name.length());
}

static android::hardware::hidl_handle wrapFd(int fd) {
    if (fd >= 0) {
        auto * handle = native_handle_create(1, 0);
        if (NULL != handle) {
            handle->data[0] = fd;
            android::hardware::hidl_handle hidl_handle;
            hidl_handle.setTo(handle, true /*take ownership*/);
            return hidl_handle;
        }
    }
    return {};
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_secuidequeue
  (JNIEnv * env, jclass /*cls*/,jint id, jint payload, jbyteArray input, jbyteArray remote)
{

    int32_t ret = 0;
    secdisp_ip ip = {};
    secdisp_op op = {};
    std::string remote_name;
    uint32_t ionFd;

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) return ret;

    op.status = sec_ui_dequeue((uintptr_t *)&op.bufHandle, &ionFd, &op.u32Stride, &op.u32PixelWidth);

    if (op.status)
    {
        return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                remote_name.length());
    }

    if(ip.bToRender) {
        op.status = 0xf;
        auto native_handle = wrapFd(ionFd);
        op.fd = native_handle;

#ifndef TARGET_ION_ABI_VERSION
        if (secui_ION_increment_ref_count(ionFd, &allocatedBuffers[nextBufferToPass].ref_ion_handle) < 0) {
            return -1;
        }
#endif

        allocatedBuffers[nextBufferToPass].handle = op.bufHandle;
        allocatedBuffers[nextBufferToPass].native_handle = std::move(native_handle);
        nextBufferToPass  = (nextBufferToPass + 1) % allocatedBuffers.size();
    }
    return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_secuienqueue
  (JNIEnv * env, jclass /*cls*/,jint id, jint payload, jbyteArray input, jbyteArray remote)
{
    int32_t ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) return ret;


    op.status = sec_ui_enqueue((uintptr_t) ip.bufHandle, (uint8_t) ip.bToRender);

    return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_stopdisp
  (JNIEnv * env, jclass /*cls*/,jint id, jint payload, jbyteArray input, jbyteArray remote)
{

    int32_t ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) return ret;

    /* release the allocated buffers of this session */
#ifndef TARGET_ION_ABI_VERSION
    for (auto& alloc_buffer: allocatedBuffers) {
        secui_ION_decrement_ref_count(&alloc_buffer.ref_ion_handle);
    }
#endif
    allocatedBuffers.clear();

    close(g_ion_dev_fd);
    op.status = sec_ui_stop_disp();

    return svc_sock_send_response_getdisp(id, payload, &op, remote_name.c_str(),
                                          remote_name.length());
 }
}
