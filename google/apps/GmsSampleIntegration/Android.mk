###############################################################################
# Various GMS Sample Integration targets
LOCAL_PATH:= $(call my-dir)

# GmsSampleIntegration
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsSampleIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_full $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsSampleIntegrationGo
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsSampleIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType1Integration (No GSA, No Chrome)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType1Integration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType1IntegrationGo (No GSA, No Chrome)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType1IntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType2Integration (No GSA, Chrome)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType2Integration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_gsa $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType2IntegrationGo (No GSA, Chrome)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType2IntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_search_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType3aIntegration (GSA, No Chrome, no placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType3aIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType3aIntegrationGo (GSA, No Chrome, no placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType3aIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType3bIntegration (GSA, No Chrome, search placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType3bIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_chrome $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType3bIntegrationGo (GSA, No Chrome, search placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType3bIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_chrome_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4aIntegration (GSA, Chrome, no placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4aIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4aIntegrationGo (GSA, Chrome, no placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4aIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_min_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4bIntegration (GSA, Chrome, search placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4bIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_chrome $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4bIntegrationGo (GSA, Chrome, search placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4bIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_no_chrome_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4cIntegration (GSA, Chrome, search+chrome placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4cIntegration
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_full $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)

# GmsEEAType4cIntegrationGo (GSA, Chrome, search+chrome placement)
include $(CLEAR_VARS)
LOCAL_PACKAGE_NAME := GmsEEAType4cIntegrationGo
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res_dhs_go $(LOCAL_PATH)/res
LOCAL_SDK_VERSION := current
include $(BUILD_PACKAGE)
