ifneq ($(TARGET_BOARD_AUTO),true)
ifeq ($(call is-board-platform-in-list,msm8998 sdm660 sdm845 sdm710 msmnile $(MSMSTEPPE) $(TRINKET)),true)

include $(call all-subdir-makefiles)

endif # end filter
endif
