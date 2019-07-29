LOCAL_PATH := $(call my-dir)

# init.qti.manifest_sku.sh
include $(CLEAR_VARS)
LOCAL_MODULE       := init.qti.manifest_sku.sh
LOCAL_SRC_FILES    := init.qti.manifest_sku.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_INIT_RC      := manifest_sku.rc
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_PREBUILT)