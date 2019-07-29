ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,sdm845),true)

include $(call all-subdir-makefiles)

endif
endif

