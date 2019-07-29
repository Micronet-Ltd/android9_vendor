ifeq ($(call is-board-platform-in-list, msm8937 msm8953 msm8996 msm8998 apq8098_latv sdm660 sdm845 sdm710 msmnile $(MSMSTEPPE) $(TRINKET)),true)
  include $(call all-subdir-makefiles)
endif

