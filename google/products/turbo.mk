###############################################################################
# Please add "$(call inherit-product, vendor/google/products/turbo.mk)"
# to your product makefile to integrate Turbo app.
# Note: Turbo currently supports arm64 ABI only.
#
PRODUCT_PACKAGES += Turbo

# Overlay for Turbo
PRODUCT_PACKAGE_OVERLAYS += vendor/google/apps/Turbo/turbo_overlay
