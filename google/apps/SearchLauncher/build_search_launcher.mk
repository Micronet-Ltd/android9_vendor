LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES += \
    android-support-v4 \
    android-support-v7-recyclerview \
    android-support-dynamic-animation \
    lib_launcherClient

LOCAL_SRC_FILES += \
    $(call all-java-files-under, src) \
    $(call all-java-files-under, $(LAUNCHER_PATH)/src) \
    $(call all-proto-files-under, $(LAUNCHER_PATH)/protos) \
    $(call all-proto-files-under, $(LAUNCHER_PATH)/proto_overrides)

LOCAL_RESOURCE_DIR += \
    $(LOCAL_PATH)/$(LAUNCHER_PATH)/res \
    prebuilts/sdk/current/support/v7/recyclerview/res

LOCAL_FULL_LIBS_MANIFEST_FILES += \
	$(LOCAL_PATH)/$(LAUNCHER_PATH)/AndroidManifest-common.xml

LOCAL_PROTOC_OPTIMIZE_TYPE := nano
LOCAL_PROTOC_FLAGS := \
    --proto_path=$(LOCAL_PATH)/$(LAUNCHER_PATH)/protos/ \
    --proto_path=$(LOCAL_PATH)/$(LAUNCHER_PATH)/proto_overrides/
LOCAL_PROTO_JAVA_OUTPUT_PARAMS := enum_style=java

LOCAL_AAPT_FLAGS := \
    --auto-add-overlay \
    --extra-packages android.support.v7.recyclerview \

LOCAL_MIN_SDK_VERSION := 26

LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Home Launcher2 Launcher3 Launcher3QuickStep

LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.launcher3.*
