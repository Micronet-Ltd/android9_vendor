-include $(TOPDIR)/rules.mk

CFLAGS := -fPIE -Wall -g -MMD
CPPFLAGS := $(CFLAGS)
LDFLAGS := -pie -fPIE -pthread -lwigig_pciaccess -lwigig_utils

ifneq ($(CONFIG_TARGET_ipq)$(CONFIG_TARGET_ipq806x)$(CONFIG_TARGET_ipq807x)$(CONFIG_TARGET_ipq807x_64),)
    is_ipq = 1
endif

ifeq ($(WIGIG_3PP_BUILD), TRUE)
    #third-party build
    CROSS := $(TARGET_CROSS)
    CPPFLAGS := --sysroot=$(SYSROOT_CPP) $(CPPFLAGS)
    INCLUDE_CFLAGS += -I$(STAGING_DIR_PLATFORM)/usr/include/libnl3
    LDFLAGS += --sysroot=$(SYSROOT_CPP) -L$(STAGING_DIR_PLATFORM)/usr/lib
else ifeq ($(is_ipq), 1)
    ifneq ($(strip $(TOOLPREFIX)),)
        CROSS:=$(TOOLPREFIX)
    endif
endif

CC = $(CROSS)gcc
CXX = $(CROSS)g++

.DEFAULT_GOAL = all
PROG = wigig_logcollector

INCLUDES = \
    -I ../lib/WlctPciAcss \
    -I ../lib/inc \
    -I ../lib/utils \

LIBS = \
    -L../lib/WlctPciAcss \
    -L../lib/utils \

CPP_FILES = $(shell find . -type f -name '*.cpp')
OBJ_FILES = $(CPP_FILES:.cpp=.o)

all: $(PROG)

$(PROG): $(OBJ_FILES)
	$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	rm -rf $(PROG)
	find . -type f \( -name "*.d" -o -name "*.o" \) -delete

-include $(OBJ_FILES:%.o=%.d)
