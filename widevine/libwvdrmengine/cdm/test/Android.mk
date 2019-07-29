# ----------------------------------------------------------------
# Builds CDM Tests
#
LOCAL_PATH := $(call my-dir)

test_name := base64_test
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := buffer_reader_test
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := cdm_feature_test
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name := cdm_engine_test
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := cdm_extended_duration_test
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name := cdm_session_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := counter_metric_unittest
test_src_dir := ../metrics/test
include $(LOCAL_PATH)/unit-test.mk

test_name := crypto_session_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := device_files_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := distribution_unittest
test_src_dir := ../metrics/test
include $(LOCAL_PATH)/unit-test.mk

test_name := event_metric_unittest
test_src_dir := ../metrics/test
include $(LOCAL_PATH)/unit-test.mk

test_name := file_store_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := file_utils_unittest
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name := http_socket_test
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := initialization_data_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := license_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := license_keys_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := policy_engine_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := policy_engine_constraints_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := request_license_test
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name := service_certificate_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := timer_unittest
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name := usage_table_header_unittest
test_src_dir := ../core/test
include $(LOCAL_PATH)/unit-test.mk

test_name := value_metric_unittest
test_src_dir := ../metrics/test
include $(LOCAL_PATH)/unit-test.mk

test_name := wv_cdm_metrics_test
test_src_dir := .
include $(LOCAL_PATH)/unit-test.mk

test_name :=
test_src_dir :=
