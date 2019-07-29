LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
SPEC_HOSTS := hosts
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/ChinaMobile/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_HOSTS) $(GENERATED_PACKAGE_PATH)/ChinaMobile/system/vendor/$(SPEC_HOSTS))
