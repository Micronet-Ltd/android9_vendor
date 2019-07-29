LOCAL_PATH := $(call my-dir)

#
# Prebuilt Launcher client library Libraries
#
include $(CLEAR_VARS)
LOCAL_MODULE := lib_launcherClient
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_SRC_FILES := libs/launcher_client.jar
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_SDK_VERSION := 28
include $(BUILD_PREBUILT)

#
# Search launcher build rule
#
include $(CLEAR_VARS)

# Relative path for Launcher3 directory
LAUNCHER_PATH := ../../../../packages/apps/Launcher3

LOCAL_SRC_FILES := \
    $(call all-java-files-under, $(LAUNCHER_PATH)/src_ui_overrides) \
    $(call all-java-files-under, $(LAUNCHER_PATH)/src_flags) \

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

LOCAL_PROGUARD_FLAG_FILES := $(LAUNCHER_PATH)/proguard.flags

LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := SearchLauncher

include $(LOCAL_PATH)/build_search_launcher.mk
include $(BUILD_PACKAGE)

#
# Search launcher quickstepbuild rule
#
include $(CLEAR_VARS)

# Relative path for Launcher3 directory
LAUNCHER_PATH := ../../../../packages/apps/Launcher3

LOCAL_STATIC_JAVA_LIBRARIES := libSharedSystemUI

LOCAL_SRC_FILES := \
    $(call all-java-files-under, quickstep/src) \
    $(call all-java-files-under, $(LAUNCHER_PATH)/quickstep/src)

LOCAL_RESOURCE_DIR := \
    $(LOCAL_PATH)/quickstep/res \
    $(LOCAL_PATH)/res \
    $(LOCAL_PATH)/$(LAUNCHER_PATH)/quickstep/res

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SDK_VERSION := system_current
LOCAL_PACKAGE_NAME := SearchLauncherQuickStep

LOCAL_FULL_LIBS_MANIFEST_FILES := $(LOCAL_PATH)/$(LAUNCHER_PATH)/quickstep/AndroidManifest.xml

include $(LOCAL_PATH)/build_search_launcher.mk
include $(BUILD_PACKAGE)

# ==================================================
include $(call all-makefiles-under,$(LOCAL_PATH))
