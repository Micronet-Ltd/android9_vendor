LOCAL_PATH := $(call my-dir)

MY_LOCAL_PATH := $(LOCAL_PATH)

include $(call all-subdir-makefiles)

LOCAL_PATH := $(MY_LOCAL_PATH)

DATAMODULE_DIR := ${LOCAL_PATH}/..

DATA_HAL_DIR   := ${LOCAL_PATH}/../..

###################### Target ########################################
include $(CLEAR_VARS)

include $(DATA_HAL_DIR)/lqe/src/lqe.mk

LOCAL_MODULE               := qcrilDataModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional

LOCAL_CFLAGS               += -Wall -Werror -Wno-macro-redefined
LOCAL_CXXFLAGS             += -std=c++14
LOCAL_SRC_FILES            := $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)

LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../endpoints/src)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../authmanager/src)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../dataconnectionservice/src)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../profilehandler/src)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../lcehandler/src)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../networkservicehandler/src)
LOCAL_SRC_FILES            += $(call all-c-files-under, ../../lqe/src)

# TODO Remove once qdphandler separated out
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../qdphandler/legacy)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, ../../qdphandler/src)

#Dont compile stub.c for target build
LOCAL_SRC_FILES := $(filter-out qcril_data_stubs.c,$(LOCAL_SRC_FILES))

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcril/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcril/framework
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

# LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

# Local
LOCAL_C_INCLUDES += $(DATAMODULE_DIR)/include
LOCAL_C_INCLUDES += $(DATAMODULE_DIR)/include/legacy
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/lcehandler/inc

LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/endpoints/inc
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/authmanager/inc
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/dataconnectionservice/inc
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/profilehandler/inc
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/networkservicehandler/inc

LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/qdphandler/inc
LOCAL_C_INCLUDES += $(DATA_HAL_DIR)/qdphandler/legacy

LOCAL_C_INCLUDES += external/nanopb-c

LOCAL_CFLAGS += -DQCRIL_DATA_MODULE_DEV

LOCAL_CFLAGS += -DFEATURE_QCRIL_USE_NETCTRL
LOCAL_CFLAGS += -DRIL_REQUEST_SET_INITIAL_ATTACH_APN
LOCAL_CFLAGS += -DFEATURE_DATA_EMBMS
LOCAL_CFLAGS += -DFEATURE_DATA_LQE

LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libdsi_netctrl
LOCAL_SHARED_LIBRARIES += libril-qc-hal-qmi
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libhardware_legacy
LOCAL_SHARED_LIBRARIES += vendor.qti.hardware.data.connection@1.0

$(info Done building qcrilDataModule for target...)

include $(BUILD_STATIC_LIBRARY)

###################### Host ############################################
 include $(CLEAR_VARS)

 LOCAL_MODULE               := qcrilDataModule
 LOCAL_MODULE_OWNER         := qti
 LOCAL_PROPRIETARY_MODULE   := true
 LOCAL_MODULE_TAGS          := optional

 LOCAL_CFLAGS               += -Wall -DHOST_EXECUTABLE_BUILD -g -fprofile-arcs -ftest-coverage --coverage -fexceptions
 LOCAL_CXXFLAGS             += -std=c++14
 LOCAL_SRC_FILES            := $(call all-cpp-files-under, src)
 LOCAL_SRC_FILES            += $(call all-c-files-under, src)

 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcril/
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcril/framework/
 #LOCAL_C_EXPORT_INCLUDES += $(TARGET_OUT_HEADERS)/qcril/modules/qmi/
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
 LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
 LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
 LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

 # Local
 LOCAL_C_INCLUDES += $(DATAMODULE_DIR)/include
 LOCAL_C_INCLUDES += $(DATAMODULE_DIR)/include/legacy

 LOCAL_C_INCLUDES += external/nanopb-c

 # This is for development only
 LOCAL_CFLAGS += -DQCRIL_DATA_MODULE_DEV

 LOCAL_CFLAGS += -DFEATURE_QCRIL_USE_NETCTRL
 LOCAL_CFLAGS += -DRIL_REQUEST_SET_INITIAL_ATTACH_APN
 LOCAL_CFLAGS += -DFEATURE_DATA_EMBMS
 LOCAL_CFLAGS += -DFEATURE_DATA_LQE
 LOCAL_CFLAGS += -DQCRIL_DATA_OFFTARGET
 LOCAL_CFLAGS += -DFEATURE_DATA_LOG_STDERR
 LOCAL_CFLAGS += -DFEATURE_UNIT_TEST

 LOCAL_SHARED_LIBRARIES += libqcrilFramework
 LOCAL_SHARED_LIBRARIES += libril-qc-hal-qmi

 #$(info Done building qcrilDataModule for host...)

 #include $(BUILD_HOST_STATIC_LIBRARY)

 LOCAL_HEADER_LIBRARIES := qcrilDataModule_headers
 LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := qcrilDataModule_headers
