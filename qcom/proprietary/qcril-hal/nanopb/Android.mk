LOCAL_PATH := $(call my-dir)

IGNORED_WARNINGS := -Wno-sign-compare -Wno-unused-parameter

# nanopb_c library
# =======================================================
nanopb_c_src_files := \
	pb_decode.c \
	pb_encode.c

include $(CLEAR_VARS)

LOCAL_MODULE := libril-protobuf-c-nano-enable_malloc
LOCAL_MODULE_TAGS := optional
LOCAL_C_EXTENSION := .c
LOCAL_SRC_FILES := $(nanopb_c_src_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS := $(IGNORED_WARNINGS) -DPB_ENABLE_MALLOC

include $(BUILD_STATIC_LIBRARY)

