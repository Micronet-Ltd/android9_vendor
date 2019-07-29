LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS:=optional debug eng tests samples
LOCAL_MODULE := checksparse.py
LOCAL_SRC_FILES := checksparse.py
LOCAL_MODULE_CLASS := EXECUTABLES
# We don't need any additional suffix.
LOCAL_MODULE_SUFFIX :=
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
LOCAL_IS_HOST_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS:=optional debug eng tests samples
LOCAL_MODULE := rawprogram0.xml
ifneq (, $(filter msm8953_64_c551, $(TARGET_PRODUCT)))
LOCAL_SRC_FILES := rawprogram0_c551.xml
else
LOCAL_SRC_FILES := rawprogram0.xml
endif
LOCAL_MODULE_CLASS := EXECUTABLES
# We don't need any additional suffix.
LOCAL_MODULE_SUFFIX :=
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
LOCAL_IS_HOST_MODULE := true

include $(BUILD_PREBUILT)
