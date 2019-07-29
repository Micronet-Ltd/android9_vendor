// Copyright 2016 Google Inc. All Rights Reserved.
//
// This file contains definitions for metrics being collected throughout the
// CDM.

#ifndef WVCDM_METRICS_METRICS_GROUP_H_
#define WVCDM_METRICS_METRICS_GROUP_H_

#include <stddef.h>
#include <stdint.h>
#include <ostream>

#include "OEMCryptoCENC.h"
#include "counter_metric.h"
#include "event_metric.h"
#include "metrics.pb.h"
#include "timer_metric.h"
#include "value_metric.h"
#include "wv_cdm_types.h"

// This definition indicates that a given metric does not need timing
// stats. Example:
//
//  M_RECORD(my_metrics, my_metric_name, NO_TIME);
#define NO_TIME 0

// Used to record metric timing and additional information about a specific
// event. Assumes that a microsecond timing has been provided. Example:
//
// long total_time = 0;
// long error_code = TimeSomeOperation(&total_time);
// M_RECORD(my_metrics, my_metric_name, total_time, error_code);
#define M_RECORD(GROUP, METRIC, TIME, ...)       \
  if (GROUP) {                                   \
    (GROUP)->METRIC.Record(TIME, ##__VA_ARGS__); \
  }

// This definition automatically times an operation and records the time and
// additional information such as error code to the provided metric.
// Example:
//
//   OEMCryptoResult sts;
//   M_TIME(
//       sts = OEMCrypto_Initialize(),
//       my_metrics_collection,
//       oemcrypto_initialize_,
//       sts);
#define M_TIME(CALL, GROUP, METRIC, ...)                 \
  if (GROUP) {                                           \
    wvcdm::metrics::TimerMetric timer;                   \
    timer.Start();                                       \
    CALL;                                                \
    (GROUP)->METRIC.Record(timer.AsUs(), ##__VA_ARGS__); \
  } else {                                               \
    CALL;                                                \
  }

namespace wvcdm {
namespace metrics {

namespace {

// Short name definitions to ease AttributeHandler definitions.
// Internal namespace to help simplify declarations.
const int kErrorCodeFieldNumber =
    ::drm_metrics::Attributes::kErrorCodeFieldNumber;
const int kErrorCodeBoolFieldNumber =
    ::drm_metrics::Attributes::kErrorCodeBoolFieldNumber;
const int kCdmSecurityLevelFieldNumber =
    ::drm_metrics::Attributes::kCdmSecurityLevelFieldNumber;
const int kSecurityLevelFieldNumber =
    ::drm_metrics::Attributes::kSecurityLevelFieldNumber;
const int kLengthFieldNumber =
    ::drm_metrics::Attributes::kLengthFieldNumber;
const int kEncryptAlgorithmFieldNumber =
    ::drm_metrics::Attributes::kEncryptionAlgorithmFieldNumber;
const int kSigningAlgorithmFieldNumber =
    ::drm_metrics::Attributes::kSigningAlgorithmFieldNumber;
const int kOemCryptoResultFieldNumber =
    ::drm_metrics::Attributes::kOemCryptoResultFieldNumber;
const int kKeyStatusTypeFieldNumber =
    ::drm_metrics::Attributes::kKeyStatusTypeFieldNumber;
const int kEventTypeFieldNumber =
    ::drm_metrics::Attributes::kEventTypeFieldNumber;
const int kKeyRequestTypeFieldNumber =
    ::drm_metrics::Attributes::kKeyRequestTypeFieldNumber;

}  // anonymous namespace

// This enum defines the conditions encountered during OEMCrypto Initialization
// in oemcrypto_adapter_dynamic.
typedef enum OEMCryptoInitializationMode {
  OEMCrypto_INITIALIZED_USING_IN_APP = 0,
  OEMCrypto_INITIALIZED_FORCING_L3 = 1,
  OEMCrypto_INITIALIZED_USING_L3_NO_L1_LIBRARY_PATH = 2,
  OEMCrypto_INITIALIZED_USING_L3_L1_OPEN_FAILED = 3,
  OEMCrypto_INITIALIZED_USING_L3_L1_LOAD_FAILED = 4,
  OEMCrypto_INITIALIZED_USING_L3_COULD_NOT_INITIALIZE_L1 = 5,
  OEMCrypto_INITIALIZED_USING_L3_WRONG_L1_VERSION = 6,
  OEMCrypto_INITIALIZED_USING_L1_WITH_KEYBOX = 7,
  OEMCrypto_INITIALIZED_USING_L1_WITH_CERTIFICATE = 8,
  OEMCrypto_INITIALIZED_USING_L1_CERTIFICATE_MIX = 9,
  OEMCrypto_INITIALIZED_USING_L3_BAD_KEYBOX = 10,
  OEMCrypto_INITIALIZED_USING_L3_COULD_NOT_OPEN_FACTORY_KEYBOX = 11,
  OEMCrypto_INITIALIZED_USING_L3_COULD_NOT_INSTALL_KEYBOX = 12,
  OEMCrypto_INITIALIZED_USING_L1_INSTALLED_KEYBOX = 13,
  OEMCrypto_INITIALIZED_USING_L3_INVALID_L1 = 14,
  OEMCrypto_INITIALIZED_USING_L1_WITH_PROVISIONING_3_0 = 15,
  OEMCrypto_INITIALIZED_L3_INITIALIZATION_FAILED = 16
} OEMCryptoInitializationMode;


// This class contains metrics for Crypto Session and OEM Crypto.
class CryptoMetrics {
 public:
  void Serialize(drm_metrics::WvCdmMetrics::CryptoMetrics *crypto_metrics)
      const;

  /* CRYPTO SESSION */
  // TODO(blueeyes): Convert this to crypto_session_default_security_level_.
  ValueMetric<CdmSecurityLevel> crypto_session_security_level_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      crypto_session_delete_all_usage_reports_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      crypto_session_delete_multiple_usage_information_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kLengthFieldNumber,
              Pow2Bucket, kEncryptAlgorithmFieldNumber, CdmEncryptionAlgorithm>
      crypto_session_generic_decrypt_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kLengthFieldNumber,
              Pow2Bucket, kEncryptAlgorithmFieldNumber, CdmEncryptionAlgorithm>
      crypto_session_generic_encrypt_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kLengthFieldNumber,
              Pow2Bucket, kSigningAlgorithmFieldNumber, CdmSigningAlgorithm>
      crypto_session_generic_sign_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kLengthFieldNumber,
              Pow2Bucket, kSigningAlgorithmFieldNumber, CdmSigningAlgorithm>
      crypto_session_generic_verify_;
  CounterMetric<kErrorCodeBoolFieldNumber, bool>
      crypto_session_get_device_unique_id_;
  CounterMetric<kErrorCodeBoolFieldNumber, bool> crypto_session_get_token_;
  ValueMetric<double> crypto_session_life_span_;
  EventMetric<kErrorCodeBoolFieldNumber, bool>
      crypto_session_load_certificate_private_key_;
  // This uses the requested security level.
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kSecurityLevelFieldNumber,
              SecurityLevel>
      crypto_session_open_;
  ValueMetric<uint32_t> crypto_session_system_id_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      crypto_session_update_usage_information_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      crypto_session_update_usage_entry_;
  ValueMetric<bool> crypto_session_usage_information_support_;
  /* OEMCRYPTO */
  ValueMetric<uint32_t> oemcrypto_api_version_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_close_session_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_copy_buffer_;
  ValueMetric<OEMCrypto_HDCP_Capability> oemcrypto_current_hdcp_capability_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_deactivate_usage_entry_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_decrypt_cenc_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_delete_usage_entry_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_delete_usage_table_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_derive_keys_from_session_key_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_force_delete_usage_entry_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_generate_derived_keys_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_generate_nonce_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generate_rsa_signature_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generate_signature_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generic_decrypt_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generic_encrypt_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generic_sign_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_generic_verify_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_get_device_id_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult, kLengthFieldNumber,
              Pow2Bucket>
      oemcrypto_get_key_data_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_get_oem_public_certificate_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_get_random_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_initialize_;
  ValueMetric<bool> oemcrypto_is_anti_rollback_hw_present_;
  ValueMetric<bool> oemcrypto_is_keybox_valid_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_load_device_rsa_key_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_load_entitled_keys_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_load_keys_;
  ValueMetric<OEMCrypto_HDCP_Capability> oemcrypto_max_hdcp_capability_;
  ValueMetric<size_t> oemcrypto_max_number_of_sessions_;
  ValueMetric<size_t> oemcrypto_number_of_open_sessions_;
  ValueMetric<OEMCrypto_ProvisioningMethod> oemcrypto_provisioning_method_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_refresh_keys_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_report_usage_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_rewrap_device_rsa_key_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_rewrap_device_rsa_key_30_;
  ValueMetric<uint16_t> oemcrypto_security_patch_level_;
  EventMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_select_key_;
  ValueMetric<CdmUsageSupportType> oemcrypto_usage_table_support_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_update_usage_table_;
  CounterMetric<kOemCryptoResultFieldNumber, OEMCryptoResult>
      oemcrypto_update_usage_entry_;
};

// This class contains session-scoped metrics. All properties and
// statistics related to operations within a single session are
// recorded here.
class SessionMetrics {
 public:
  SessionMetrics();

  // Sets the session id of the metrics group. This allows the session
  // id to be captured and reported as part of the collection of metrics.
  void SetSessionId(const CdmSessionId &session_id) {
    session_id_ = session_id;
  }

  // Returns the session id or an empty session id if it has not been set.
  const CdmSessionId &GetSessionId() const { return session_id_; }

  // Marks the metrics object as completed and ready for serialization.
  void SetCompleted() { completed_ = true; }

  // Returns true if the object is completed. This is used to determine
  // when the stats are ready to be published.
  bool IsCompleted() const { return completed_; }

  // Returns a pointer to the crypto metrics belonging to the engine instance.
  // This instance retains ownership of the object.
  CryptoMetrics *GetCryptoMetrics() { return &crypto_metrics_; }

  // Metrics collected at the session level.
  ValueMetric<double> cdm_session_life_span_;  // Milliseconds.
  EventMetric<kErrorCodeFieldNumber, CdmResponseType> cdm_session_renew_key_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_session_restore_offline_session_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_session_restore_usage_session_;

  EventMetric<kKeyRequestTypeFieldNumber, CdmKeyRequestType>
      cdm_session_license_request_latency_ms_;

  // Serialize the session metrics to the provided |metric_group|.
  // |metric_group| is owned by the caller and must not be null.
  void Serialize(drm_metrics::WvCdmMetrics::SessionMetrics *session_metrics)
      const;

 private:
  void SerializeSessionMetrics(
      drm_metrics::WvCdmMetrics::SessionMetrics *session_metrics) const;
  CdmSessionId session_id_;
  bool completed_;
  CryptoMetrics crypto_metrics_;
};

// This class contains metrics for the OEMCrypto Dynamic Adapter. They are
// separated from other metrics because they need to be encapsulated in a
// singleton object. This is because the dynamic adapter uses the OEMCrypto
// function signatures and contract and cannot be extended to inject
// dependencies.
//
// Operations for this metrics class are serialized since these particular
// metrics may be accessed by a separate thread during intialize even as
// the metric may be serialized.
class OemCryptoDynamicAdapterMetrics {
 public:
  explicit OemCryptoDynamicAdapterMetrics();

  // Set methods for OEMCrypto metrics.
  void SetInitializationMode(OEMCryptoInitializationMode mode);
  void SetL1ApiVersion(uint32_t version);
  void SetL1MinApiVersion(uint32_t version);

  // Serialize the session metrics to the provided |metric_group|.
  // |metric_group| is owned by the caller and must not be null.
  void Serialize(drm_metrics::WvCdmMetrics::EngineMetrics *engine_metrics)
      const;

  // Clears the existing metric values.
  void Clear();

 private:
  mutable Lock adapter_lock_;
  ValueMetric<OEMCryptoInitializationMode> oemcrypto_initialization_mode_;
  ValueMetric<uint32_t> oemcrypto_l1_api_version_;
  ValueMetric<uint32_t> oemcrypto_l1_min_api_version_;
};

// This will fetch the singleton instance for dynamic adapter metrics.
// This method is safe only if we use C++ 11. In C++ 11, static function-local
// initialization is guaranteed to be threadsafe. We return the reference to
// avoid non-guaranteed destructor order problems. Effectively, the destructor
// is never run for the created instance.
OemCryptoDynamicAdapterMetrics &GetDynamicAdapterMetricsInstance();

// This class contains engine-scoped metrics. All properties and
// statistics related to operations within the engine, but outside
// the scope of a session are recorded here.
class EngineMetrics {
 public:
  EngineMetrics();
  ~EngineMetrics();

  // Add a new SessionMetrics instance and return a pointer to the caller.
  // The new SessionMetrics instance is owned by this EngineMetrics instance
  // and will exist until RemoveSession is called or this object is deleted.
  SessionMetrics *AddSession();

  // Removes the metrics object for the given session id. This should only
  // be called when the SessionMetrics instance is no longer in use.
  void RemoveSession(CdmSessionId session_id);

  // Returns a pointer to the crypto metrics belonging to the engine instance.
  // The CryptoMetrics instance is still owned by this object and will exist
  // until this object is deleted.
  CryptoMetrics *GetCryptoMetrics() { return &crypto_metrics_; }

  // Serialize engine and session metrics into a serialized MetricsGroup
  // instance and output that instance to the provided |metric_group|.
  // |metric_group| is owned by the caller and must NOT be null.
  // |completed_only| indicates that this call should only publish
  //     SessionMetrics instances that are marked as completed.
  // |clear_sessions| indicates that this call should clear sessions metrics
  //     for those sessions that were serialized. This allows atomic
  //     serialization and closing of session-level metrics.
  // void Serialize(drm_metrics::MetricsGroup* metric_group, bool
  // completed_only,
  //               bool clear_serialized_sessions);
  void Serialize(drm_metrics::WvCdmMetrics *engine_metrics) const;

  void SetAppPackageName(const std::string &app_package_name);

  // Metrics recorded at the engine level.
  EventMetric<kErrorCodeFieldNumber, CdmResponseType> cdm_engine_add_key_;
  ValueMetric<std::string> cdm_engine_cdm_version_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_close_session_;
  ValueMetric<int64_t> cdm_engine_creation_time_millis_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType, kLengthFieldNumber,
              Pow2Bucket>
      cdm_engine_decrypt_;
  CounterMetric<kErrorCodeBoolFieldNumber, bool>
      cdm_engine_find_session_for_key_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_generate_key_request_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_get_provisioning_request_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_get_secure_stop_ids_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_get_usage_info_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_handle_provisioning_response_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_open_key_set_session_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_open_session_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_query_key_status_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_release_all_usage_info_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_release_usage_info_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_remove_all_usage_info_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType> cdm_engine_remove_keys_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType>
      cdm_engine_remove_usage_info_;
  EventMetric<kErrorCodeFieldNumber, CdmResponseType> cdm_engine_restore_key_;
  CounterMetric<kErrorCodeFieldNumber, CdmResponseType,
                kCdmSecurityLevelFieldNumber, CdmSecurityLevel>
      cdm_engine_unprovision_;

 private:
  mutable Lock session_metrics_lock_;
  std::vector<metrics::SessionMetrics *> session_metrics_list_;
  // This is used to populate the engine lifespan metric
  metrics::TimerMetric life_span_internal_;
  CryptoMetrics crypto_metrics_;
  std::string app_package_name_;

  void SerializeEngineMetrics(
      drm_metrics::WvCdmMetrics::EngineMetrics *engine_metrics) const;
};

}  // namespace metrics
}  // namespace wvcdm

#endif  // WVCDM_METRICS_METRICS_GROUP_H_
