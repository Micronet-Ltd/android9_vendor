ifneq ($(TARGET_NO_TELEPHONY), true)
LOCAL_DIR := $(call my-dir)

qcril_cflags += -O0 -g
ifeq ($(TARGET_USES_QCOM_BSP_ATEL),true)
qcril_cflags += -DTARGET_USES_QCOM_BSP_ATEL
else ifeq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
qcril_cflags += -DTARGET_USES_QCOM_BSP_ATEL
endif
qcril_cppflags += -O0 -g
qcril_ldflags += -Wl,--no-allow-shlib-undefined,--unresolved-symbols=report-all
ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
qcril_sanitize := address
endif

ifeq ($(ATEL_ENABLE_LLVM_SA),true)
    LLVM_SA_OUTPUT_DIR := $(PRODUCT_OUT)/atel-llvm-sa-results/qcril-hal
    LLVM_SA_FLAG := --compile-and-analyze $(LLVM_SA_OUTPUT_DIR)
    qcril_cflags   += $(LLVM_SA_FLAG)
    qcril_cppflags += $(LLVM_SA_FLAG)
endif

include $(call all-makefiles-under,$(LOCAL_DIR))
endif
