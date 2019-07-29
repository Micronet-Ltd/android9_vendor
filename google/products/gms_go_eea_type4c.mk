$(call inherit-product, vendor/google/products/eea_go_common.mk)

PRODUCT_COPY_FILES += \
    vendor/google/etc/sysconfig/eea_search_chrome.xml:system/etc/sysconfig/eea_search_chrome.xml

PRODUCT_PACKAGE_OVERLAYS += vendor/google/products/assistant_go_overlay

PRODUCT_PACKAGES += \
    AssistantGo \
    GoogleSearchGo \
    Chrome \
    GmsEEAType4cIntegrationGo \
    Launcher3Go
