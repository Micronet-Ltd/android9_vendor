LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))
include $(call all-subdir-makefiles)
endif
