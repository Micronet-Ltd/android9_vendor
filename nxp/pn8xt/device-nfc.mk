#####
##### NXP NFC Device Configuration makefile
######

NXP_NFC_FW_LIB64 := true
DEVICE_MANIFEST_FILE += vendor/nxp/pn8xt/qc/manifest.xml

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:system/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml 

PRODUCT_COPY_FILES += \
    vendor/nxp/pn8xt/conf/libnfc-nci.conf:vendor/etc/libnfc-nci.conf \
	vendor/nxp/pn8xt/conf/libnfc-nxp.conf:vendor/etc/libnfc-nxp.conf \
    vendor/nxp/pn8xt/conf/libnfc-nxp_RF.conf:vendor/etc/libnfc-nxp_RF.conf

######NXP_NFC_FW_LIB
ifeq ($(NXP_NFC_FW_LIB64),true)
PRODUCT_COPY_FILES += \
    vendor/nxp/pn8xt/FW/libpn553_fw_11_01_18_prod_Eval1_SLALM_CFG2_EFM_40x20_64bits.so:vendor/firmware/libpn553_fw.so
else
PRODUCT_COPY_FILES += \
    vendor/nxp/pn8xt/FW/libpn553_fw_11_01_18_prod_Eval1_SLALM_CFG2_EFM_40x20_32bits.so:vendor/firmware/libpn553_fw.so
endif

# NFC Init Files
PRODUCT_COPY_FILES += \
     vendor/nxp/pn8xt/hw/init.pn8x.nfc.rc:vendor/etc/init/init.pn8x.nfc.rc \
     vendor/nxp/pn8xt/hw/init.pn8x.se.rc:vendor/etc/init/init.pn8x.se.rc 

# NFC packages
PRODUCT_PACKAGES += \
    libnfc-nci \
	libnfc_nci_jni \
	nfc_nci_nxp \
	com.nxp.nfc \
    NfcNci \
    Tag \
    android.hardware.nfc@1.0-impl \
	android.hardware.nfc@1.1-service 
    
    

#PRODUCT_PACKAGES += \
#	android.hardware.secure_element@1.0-service \
#	android.hardware.wired_se@1.0-service \

# SE config files
#PRODUCT_COPY_FILES += \
#    hardware/nxp/secure_element/libese-spi/p73/libese-nxp-P73.conf:vendor/etc/libese-nxp.conf

BOARD_SEPOLICY_DIRS += vendor/nxp/pn8xt/sepolicy \
                       vendor/nxp/pn8xt/sepolicy/nfc \
                       vendor/nxp/pn8xt/sepolicy/se \
                       vendor/nxp/pn8xt/sepolicy/wiredse \
