$(call inherit-product, vendor/google/products/eea_common.mk)

PRODUCT_COPY_FILES += \
    vendor/google/etc/sysconfig/eea_search.xml:system/etc/sysconfig/eea_search.xml

PRODUCT_PACKAGE_OVERLAYS += vendor/google/products/assistant_gsa_overlay

PRODUCT_PACKAGES += \
    Velvet \
    GmsEEAType3bIntegration \
    SearchLauncherQuickStep
