$(call inherit-product, vendor/google/products/eea_go_common.mk)

PRODUCT_COPY_FILES += \
    vendor/google/etc/sysconfig/eea.xml:system/etc/sysconfig/eea.xml

PRODUCT_PACKAGE_OVERLAYS += vendor/google/products/assistant_vas_overlay

PRODUCT_PACKAGES += \
    Chrome \
    GoogleActionsService \
    GmsEEAType2IntegrationGo \
    Launcher3GoNoQsb
