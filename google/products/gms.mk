###############################################################################
# GMS home folder location
# Note: we cannot use $(my-dir) in this makefile
ANDROID_PARTNER_GMS_HOME := vendor/google

# GMS mandatory core packages
PRODUCT_PACKAGES := \
    AndroidPlatformServices \
    ConfigUpdater \
    GoogleBackupTransport \
    GoogleExtServices \
    GoogleExtShared \
    GoogleFeedback \
    GoogleOneTimeInitializer \
    GooglePackageInstaller \
    GooglePartnerSetup \
    GooglePrintRecommendationService \
    GoogleRestore \
    GoogleServicesFramework \
    GoogleCalendarSyncAdapter \
    GoogleContactsSyncAdapter \
    GoogleTTS \
    GmsCore \
    Phonesky \
    SetupWizard \
    Wellbeing \
    WebViewGoogle

# GMS mandatory libraries
PRODUCT_PACKAGES += com.google.android.maps.jar

# Overlay for GMS devices
$(call inherit-product, device/sample/products/backup_overlay.mk)
$(call inherit-product, device/sample/products/location_overlay.mk)
PRODUCT_PACKAGE_OVERLAYS += \
    $(ANDROID_PARTNER_GMS_HOME)/products/assistant_gsa_overlay \
    $(ANDROID_PARTNER_GMS_HOME)/products/gms_overlay \
    $(ANDROID_PARTNER_GMS_HOME)/products/webview_overlay
PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(ANDROID_PARTNER_GMS_HOME)/products/webview_overlay

# Overlay for GoogleDialer
#PRODUCT_PACKAGE_OVERLAYS += $(ANDROID_PARTNER_GMS_HOME)/apps/GoogleDialer/overlay

# Configuration files for GMS apps
PRODUCT_COPY_FILES += \
    $(ANDROID_PARTNER_GMS_HOME)/etc/default-permissions/default-permissions-google.xml:system/etc/default-permissions/default-permissions-google.xml \
    $(ANDROID_PARTNER_GMS_HOME)/etc/permissions/privapp-permissions-google.xml:system/etc/permissions/privapp-permissions-google.xml \
    $(ANDROID_PARTNER_GMS_HOME)/etc/preferred-apps/google.xml:system/etc/preferred-apps/google.xml \
    $(ANDROID_PARTNER_GMS_HOME)/etc/sysconfig/google.xml:system/etc/sysconfig/google.xml \
    $(ANDROID_PARTNER_GMS_HOME)/etc/sysconfig/google-hiddenapi-package-whitelist.xml:system/etc/sysconfig/google-hiddenapi-package-whitelist.xml

# GMS mandatory application packages
PRODUCT_PACKAGES += \
    Chrome \
    Drive \
    Gmail2 \
    Duo \
    Maps \
    Music2 \
    Photos \
    Velvet \
    Videos \
    YouTube

# GMS comms suite
PRODUCT_PACKAGES += \
    CarrierServices 
    #GoogleContacts 
    #Messages
    #GoogleDialer \

# GMS optional application packages
PRODUCT_PACKAGES += \
    CalendarGoogle \
    DeskClockGoogle \
    FaceLock \
    LatinImeGoogle \
    TagGoogle \
    talkback \
    Keep \
    CalculatorGoogle \
    OobConfig

# Overlay for SearchLauncherQuickStep
PRODUCT_PACKAGE_OVERLAYS += \
    $(ANDROID_PARTNER_GMS_HOME)/apps/SearchLauncher/quickstep/overlay \
    $(ANDROID_PARTNER_GMS_HOME)/apps/SearchLauncher/quickstep/static_overlay
PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(ANDROID_PARTNER_GMS_HOME)/apps/SearchLauncher/quickstep/static_overlay

# GMS sample application packages
PRODUCT_PACKAGES += \
    GmsSampleIntegration \
    SearchLauncherQuickStep

# Overrides
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    ro.opa.eligible_device=true \
    ro.setupwizard.mode=OPTIONAL \
    setupwizard.feature.predeferred_enabled=false \
    ro.com.google.gmsversion=9_201906
