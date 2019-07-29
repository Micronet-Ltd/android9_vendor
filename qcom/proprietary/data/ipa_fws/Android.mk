TARGET_LIST := msm8998 sdm845 sdm710 msmnile $(MSMSTEPPE) $(TRINKET)

LOCAL_PATH := $(call my-dir)
PILSPLITTER_BASE := vendor/qcom/proprietary/common/scripts
HASH_SEG_ALGO := sha384

ifeq ($(call is-board-platform-in-list,$(TARGET_LIST)),true)

# Firmware hardware addresses
# If these differ across devices, define them accordingly.
DPS_ADDR := 0x01E5E000
HPS_ADDR := 0x01E5E080
GSI_ADDR := 0x01E08000

# msm8998 IPA FWs configs (IPAv3.1)
ifeq ($(call is-board-platform-in-list,msm8998),true)
$(info IPA FW creation for msm8998)
GSI_FW_FILE := fw_mhi_ipa_v3.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x30020000
# For 8997 HW version
SOC_VERS := 0x3005
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
endif #msm8998 check

# sdm845 IPA FWs configs (IPAv3.5.1)
ifeq ($(call is-board-platform-in-list,sdm845),true)
$(info IPA FW creation for sdm845)
GSI_FW_FILE := fw_mhi_ipa_v3.5.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60000000
SOC_VERS := 0x6001
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
endif #sdm845 check


# sdm710 IPA FWs configs (IPAv3.5.1)
ifeq ($(call is-board-platform-in-list,sdm710),true)
$(info IPA FW creation for sdm710)
GSI_FW_FILE := fw_mhi_ipa_v3.5.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60040000
SOC_VERS := 0x6005 0x6009 0x600A
SECIMAGE_CONFIG_FILE := config/integration/secimagev2.xml
SECIMAGE_BASE := vendor/qcom/proprietary/common/scripts/SecImage
endif #sdm710 check

# msmnile IPA FWs configs (IPAv4.1)
ifeq ($(call is-board-platform-in-list,msmnile),true)
$(info IPA FW creation for msmnile)
GSI_FW_FILE := fw_mhi_ipa_v4.0.bin
GSI_MEM_SIZE := 0x6000
SOC_HW_VERSION := 0x60030100
SOC_VERS := 0x6003
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #msmnile check

# MSMSTEPPE IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,$(MSMSTEPPE)),true)
$(info IPA FW creation for $(MSMSTEPPE))
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x60070000
SOC_VERS := "0x6007 0x600c"
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #MSMSTEPPE check

# TRINKET IPA FWs configs (IPAv4.2)
ifeq ($(call is-board-platform-in-list,$(TRINKET)),true)
$(info IPA FW creation for $(TRINKET))
DPS_ADDR := 0x0585E000
HPS_ADDR := 0x0585E080
GSI_ADDR := 0x05808000
GSI_FW_FILE := fw_mhi_ipa_v4.2.bin
GSI_MEM_SIZE := 0x4000
SOC_HW_VERSION := 0x90010100
SOC_VERS := 0x9001
SECIMAGE_CONFIG_FILE := config/integration/secimagev3.xml
ifeq ("$(wildcard vendor/qcom/proprietary/sectools)","")
SECIMAGE_BASE := vendor/qcom/proprietary/tools/sectools
else
SECIMAGE_BASE := vendor/qcom/proprietary/sectools
endif
endif #TRINKET check

# Check GSI FW file existence
ifeq ("$(wildcard $(LOCAL_PATH)/$(GSI_FW_FILE))","")
$(info $(LOCAL_PATH)/$(GSI_FW_FILE) does not exist!)
else

BINS := $(LOCAL_PATH)/$(GSI_FW_FILE) $(LOCAL_PATH)/hps.bin $(LOCAL_PATH)/dps.bin
TOOLS := $(LOCAL_PATH)/elf_creator.py $(SECIMAGE_BASE)/sectools_builder.py $(PILSPLITTER_BASE)/pil-splitter.py
OUT_IPA_FW := $(OUT_DIR)/ipa_tmp
PRIVATE_PATH := $(LOCAL_PATH)

$(TARGET_OUT_VENDOR)/firmware/ipa_fws.elf: $(BINS) $(TOOLS)
	rm -rf $(TARGET_OUT_VENDOR)/firmware/ipa_fws.elf

include $(CLEAR_VARS)
LOCAL_MODULE := hps.bin
ifeq ("$(wildcard $(TARGET_OUT_VENDOR)/firmware/ipa_fws.elf)","")
$(info $(TARGET_OUT_VENDOR)/firmware/ipa_fws.elf does not exist!)
$(info $(shell mkdir -p $(OUT_IPA_FW)))
# Assemble binaries/firmwares to a single ELF file
$(info $(shell python $(PRIVATE_PATH)/elf_creator.py \
		--gsi_fw $(PRIVATE_PATH)/$(GSI_FW_FILE) \
		--gsi_fw_address $(GSI_ADDR) \
		--gsi_fw_mem_size $(GSI_MEM_SIZE) \
		--hps $(PRIVATE_PATH)/hps.bin \
		--hps_addr $(HPS_ADDR) \
		--dps $(PRIVATE_PATH)/dps.bin \
		--dps_addr $(DPS_ADDR) \
		--outfile $(OUT_IPA_FW)/ipa_fws.elf))
$(info $(shell echo Creating ipa ELF image OUT folders))
$(info $(shell mkdir -p $(PRODUCT_OUT)/ipa))
$(info $(shell mkdir -p $(PRODUCT_OUT)/ipa/signed))
$(info $(shell mkdir -p $(PRODUCT_OUT)/ipa/unsigned))
$(info $(shell echo install unsigned ipa_fws.elf at $(PRODUCT_OUT)/ipa/unsigned))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.elf $(PRODUCT_OUT)/ipa/unsigned))
# Sign the ELF file using SecImage tool
$(info $(shell SECIMAGE_LOCAL_DIR=$(SECIMAGE_BASE) USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN=$(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN) \
	python $(SECIMAGE_BASE)/sectools_builder.py \
		-i $(OUT_IPA_FW)/ipa_fws.elf \
		-g ipa_fw \
		-t $(OUT_IPA_FW) \
		--install_base_dir=$(OUT_IPA_FW) \
		--config $(SECIMAGE_BASE)/$(SECIMAGE_CONFIG_FILE) \
		--soc_hw_version=$(SOC_HW_VERSION) \
		--soc_vers=$(SOC_VERS) \
		> $(OUT_IPA_FW)/secimage.log 2>&1))
$(info $(shell echo install signed ipa_fws.elf at $(PRODUCT_OUT)/ipa/signed))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.elf $(PRODUCT_OUT)/ipa/signed))
# PIL split the output of the SecImage tool
$(info $(shell python $(PILSPLITTER_BASE)/pil-splitter.py \
		$(OUT_IPA_FW)/ipa_fws.elf \
		$(OUT_IPA_FW)/ipa_fws))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.elf $(TARGET_OUT_VENDOR)/firmware))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.b00 $(TARGET_OUT_VENDOR)/firmware))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.b01 $(TARGET_OUT_VENDOR)/firmware))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.b02 $(TARGET_OUT_VENDOR)/firmware))
# Only if the policy is defined, these files will be created
ifeq ($(USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN),1)
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.b03 $(TARGET_OUT_VENDOR)/firmware))
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.b04 $(TARGET_OUT_VENDOR)/firmware))
endif # Signing policy check
$(info $(shell cp -f $(OUT_IPA_FW)/ipa_fws.mdt $(TARGET_OUT_VENDOR)/firmware))
$(info $(shell rm -rf $(OUT_IPA_FW)))
else
$(info $(TARGET_OUT_VENDOR)/firmware/ipa_fws.elf didn't change!)
endif # ipa_fws.els existance check
LOCAL_MODULE_OWNER := ipa
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(OUT_IPA_FW)
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif # GSI FW file existence check
endif # Target check
