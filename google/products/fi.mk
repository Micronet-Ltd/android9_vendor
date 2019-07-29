###############################################################################
# Only use this makefile for the Project Fi devices
$(call inherit-product, vendor/google/products/gms.mk)

# GCS and Tycho apps are mandatory for Project Fi
PRODUCT_PACKAGES += GCS Tycho

# Overlay for Fi devices
PRODUCT_PACKAGE_OVERLAYS += vendor/google/products/fi_overlay 

# Configuration files for Fi apps
PRODUCT_COPY_FILES += \
    vendor/google/etc/sysconfig/google-fi.xml:system/etc/sysconfig/google-fi.xml
