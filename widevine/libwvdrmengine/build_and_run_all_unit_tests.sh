#!/bin/sh

set -e

if [ -z "$ANDROID_BUILD_TOP" ]; then
  echo "Android build environment not set"
  exit -1
fi

# Read arguments in case the user wants to do a multicore build
NUM_CORES=1
while getopts "j:" opt; do
  case $opt in
    j)
      NUM_CORES=$OPTARG
      ;;
  esac
done

# Define the relevant aliases
. $ANDROID_BUILD_TOP/build/envsetup.sh

# Build all the targets
cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine/test/unit
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine/cdm/test
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine/mediacrypto/test
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine/mediadrm/test
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine/oemcrypto/test
pwd
mm -j $NUM_CORES || mma -j $NUM_CORES

# Detect the device and check if Verity is going to stop the script from working
echo "waiting for device"
ADB_OUTPUT=`adb root && echo ". " && adb wait-for-device remount`
echo $ADB_OUTPUT
if echo $ADB_OUTPUT | grep -qi "verity"; then
  echo
  echo "ERROR: This device has Verity enabled. build_and_run_all_unit_tests.sh "
  echo "does not work if Verity is enabled. Please disable Verity with"
  echo "\"adb disable-verity\" and try again."
  exit -1
fi

# Push the files to the device

# Given a local path to a file, this will try to push it to /data/bin.
# If that fails, an error message will be printed.
try_adb_push() {
  # android-tests.zip requires /data/bin, we should use the same
  if [ -f $OUT/data/bin/$1 ]; then
    test_file=$OUT/data/bin/$1
  else
    echo "I cannot find $1"
    echo "I think it should be in $OUT/data/bin"
    exit 1
  fi
  adb shell mkdir -p /data/bin
  adb push $test_file /data/bin/$1
}

# Push the tests to the device
try_adb_push base64_test
try_adb_push buffer_reader_test
try_adb_push cdm_engine_test
try_adb_push cdm_feature_test
try_adb_push cdm_extended_duration_test
try_adb_push cdm_session_unittest
try_adb_push counter_metric_unittest
try_adb_push crypto_session_unittest
try_adb_push device_files_unittest
try_adb_push distribution_unittest
try_adb_push event_metric_unittest
try_adb_push file_store_unittest
try_adb_push file_utils_unittest
try_adb_push http_socket_test
try_adb_push initialization_data_unittest
try_adb_push libwvdrmdrmplugin_hidl_test
try_adb_push libwvdrmdrmplugin_test
try_adb_push libwvdrmengine_hidl_test
try_adb_push libwvdrmengine_test
try_adb_push libwvdrmmediacrypto_hidl_test
try_adb_push libwvdrmmediacrypto_test
try_adb_push license_keys_unittest
try_adb_push license_unittest
try_adb_push oemcrypto_test
try_adb_push policy_engine_constraints_unittest
try_adb_push policy_engine_unittest
try_adb_push request_license_test
try_adb_push service_certificate_unittest
try_adb_push timer_unittest
try_adb_push usage_table_header_unittest
try_adb_push value_metric_unittest
try_adb_push wv_cdm_metrics_test

# Run the tests using run_all_unit_tests.sh
cd $ANDROID_BUILD_TOP/vendor/widevine/libwvdrmengine
./run_all_unit_tests.sh
