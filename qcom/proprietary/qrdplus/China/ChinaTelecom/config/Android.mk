# Disable this makefile, After verify all commands can work, please remove the ifeq condition
ifeq (1,1)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/ChinaTelecom)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(GENERATED_PACKAGE_PATH)/ChinaTelecom/$(PRE_LOAD_SPEC))

#################################################
SPEC_PROP := vendor.prop
$(shell mkdir -p $(GENERATED_PACKAGE_PATH)/ChinaTelecom/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(GENERATED_PACKAGE_PATH)/ChinaTelecom/system/vendor/$(SPEC_PROP))

ifeq (0,1)
#################################################
GPS_CONF := gps.conf
GPS_CONF_FILE := $(PRODUCT_OUT)/system/etc/$(GPS_CONF)
GPS_CONF_CT_FILE := $(TARGET_OUT)"/vendor/ChinaTelecom/system/etc"
InstallCarrierFileList := $(PRODUCT_OUT)/installed-files.txt

InstallCTGPSConf: $(InstallCarrierFileList)
	@mkdir -p "$(GPS_CONF_CT_FILE)"
	@cp -rf "$(GPS_CONF_FILE)" "$(GPS_CONF_CT_FILE)/$(GPS_CONF)"
	@if [ -f  "$(GPS_CONF_FILE).bakforspec" ] ; then \
	    cp -rf "$(GPS_CONF_FILE).bakforspec" "$(GPS_CONF_CT_FILE)/$(GPS_CONF)" ;\
	 fi
	@sed -i 's/^SUPL_VER/# &/;s/^SUPL_HOST/# &/;s/^SUPL_PORT/# &/;s/^SGLTE_TARGET/# &/' "$(GPS_CONF_CT_FILE)/$(GPS_CONF)"

InstallCarrier: InstallCTGPSConf
endif
endif
