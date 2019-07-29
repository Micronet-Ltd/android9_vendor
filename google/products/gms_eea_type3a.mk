$(call inherit-product, vendor/google/products/eea_common.mk)

PRODUCT_COPY_FILES += \
    vendor/google/etc/sysconfig/eea.xml:system/etc/sysconfig/eea.xml

PRODUCT_PACKAGE_OVERLAYS += vendor/google/products/assistant_gsa_overlay

PRODUCT_PACKAGES += \
    Velvet \
    GmsEEAType3aIntegration \
    Launcher3NoQsb
