LOCAL_PATH := $(call my-dir)

LAUNCHER_PATH := packages/apps/Launcher3
RELATIVE_LAUNCHER_PATH := ../../../../packages/apps/Launcher3

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-annotations \
    libSharedSystemUI

LOCAL_STATIC_ANDROID_LIBRARIES := \
    android-support-compat \
    android-support-media-compat \
    android-support-core-utils \
    android-support-core-ui \
    android-support-fragment \
    android-support-v7-recyclerview \
    android-support-dynamic-animation

LOCAL_SRC_FILES := \
    $(call all-java-files-under, $(RELATIVE_LAUNCHER_PATH)/src) \
    $(call all-java-files-under, $(RELATIVE_LAUNCHER_PATH)/quickstep/src) \
    $(call all-java-files-under, src_flags) \
    $(call all-proto-files-under, $(RELATIVE_LAUNCHER_PATH)/protos) \
    $(call all-proto-files-under, $(RELATIVE_LAUNCHER_PATH)/proto_overrides)

LOCAL_RESOURCE_DIR := \
    $(LAUNCHER_PATH)/quickstep/res \
    $(LAUNCHER_PATH)/res

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_PROTOC_OPTIMIZE_TYPE := nano
LOCAL_PROTOC_FLAGS := \
    --proto_path=$(LOCAL_PATH)/$(RELATIVE_LAUNCHER_PATH)/protos/ \
    --proto_path=$(LOCAL_PATH)/$(RELATIVE_LAUNCHER_PATH)/proto_overrides/
LOCAL_PROTO_JAVA_OUTPUT_PARAMS := enum_style=java

LOCAL_USE_AAPT2 := true

LOCAL_SDK_VERSION := system_current
LOCAL_MIN_SDK_VERSION := 26
LOCAL_PACKAGE_NAME := Launcher3NoQsb
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Home Launcher2 Launcher3 Launcher3QuickStep

LOCAL_FULL_LIBS_MANIFEST_FILES := \
    $(LAUNCHER_PATH)/AndroidManifest.xml \
    $(LAUNCHER_PATH)/AndroidManifest-common.xml

LOCAL_MANIFEST_FILE := $(RELATIVE_LAUNCHER_PATH)/quickstep/AndroidManifest.xml
LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.launcher3.*

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-annotations

LOCAL_STATIC_ANDROID_LIBRARIES := \
    android-support-compat \
    android-support-media-compat \
    android-support-core-utils \
    android-support-core-ui \
    android-support-fragment \
    android-support-v7-recyclerview \
    android-support-dynamic-animation

LOCAL_SRC_FILES := \
    $(call all-java-files-under, $(RELATIVE_LAUNCHER_PATH)/src) \
    $(call all-java-files-under, $(RELATIVE_LAUNCHER_PATH)/src_ui_overrides) \
    $(call all-java-files-under, go/src_flags) \
    $(call all-proto-files-under, $(RELATIVE_LAUNCHER_PATH)/protos) \
    $(call all-proto-files-under, $(RELATIVE_LAUNCHER_PATH)/proto_overrides)

LOCAL_RESOURCE_DIR := \
    $(LAUNCHER_PATH)/go/res \
    $(LAUNCHER_PATH)/res

LOCAL_PROGUARD_FLAG_FILES := $(RELATIVE_LAUNCHER_PATH)/proguard.flags

LOCAL_PROTOC_OPTIMIZE_TYPE := nano
LOCAL_PROTOC_FLAGS := \
    --proto_path=$(LOCAL_PATH)/$(RELATIVE_LAUNCHER_PATH)/protos/ \
    --proto_path=$(LOCAL_PATH)/$(RELATIVE_LAUNCHER_PATH)/proto_overrides/
LOCAL_PROTO_JAVA_OUTPUT_PARAMS := enum_style=java

LOCAL_USE_AAPT2 := true

LOCAL_SDK_VERSION := current
LOCAL_MIN_SDK_VERSION := 21
LOCAL_PACKAGE_NAME := Launcher3GoNoQsb
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Home Launcher2 Launcher3 Launcher3QuickStep Launcher3Go

LOCAL_FULL_LIBS_MANIFEST_FILES := \
    $(LAUNCHER_PATH)/AndroidManifest.xml \
    $(LAUNCHER_PATH)/AndroidManifest-common.xml

LOCAL_MANIFEST_FILE := $(RELATIVE_LAUNCHER_PATH)/go/AndroidManifest.xml
LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.launcher3.*

include $(BUILD_PACKAGE)
