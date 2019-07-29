LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/CTA)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(GENERATED_PACKAGE_PATH)/CTA/$(PRE_LOAD_SPEC))

ifeq (0,1)
#################################################
SPEC_PROP := local.prop
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/CTA/data)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(GENERATED_PACKAGE_PATH)/CTA/data/$(SPEC_PROP))
endif

#################################################
SPEC_PROP := vendor.prop
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/CTA/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(GENERATED_PACKAGE_PATH)/CTA/system/vendor/$(SPEC_PROP))
