LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src) \
    $(call all-java-files-under, wrapper) \
    src/com/android/backup/IBackupCallback.aidl \
    src/com/android/backup/IRemoteBackupService.aidl
#LOCAL_SRC_FILES += $(call all-java-files-under, ../../../../../../../../external/apache-http/src/org/apache/commons/)

LOCAL_PACKAGE_NAME := Backup

#LOCAL_SDK_VERSION := current
LOCAL_PRIVATE_PLATFORM_APIS := true

#LOCAL_JAVA_LIBRARIES += telephony-common mms-common
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += org.apache.http.legacy.boot

LOCAL_PROGUARD_ENABLED := disabled
#LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
