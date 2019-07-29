ifeq ($(call is-board-platform-in-list,msm8909 msm8916 msm8952 msm8996 msm8937 msm8953 msm8998 sdm630 sdm660 sdm710 sdm845 msmnile qcs605 $(MSMSTEPPE) trinket),true)
include $(call all-subdir-makefiles)
endif
