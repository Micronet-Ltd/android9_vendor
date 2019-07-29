ifneq ($(AUDIO_USE_STUB_HAL), true)
AUDCAL_ROOT := $(call my-dir)
include $(call all-subdir-makefiles)
endif
