// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#ifndef WVCDM_CORE_CRYPTO_SESSSION_H_
#define WVCDM_CORE_CRYPTO_SESSSION_H_

#include <map>
#include <string>
#include <vector>

#include "OEMCryptoCENC.h"
#include "key_session.h"
#include "lock.h"
#include "metrics_collections.h"
#include "oemcrypto_adapter.h"
#include "scoped_ptr.h"
#include "timer_metric.h"
#include "wv_cdm_types.h"

namespace wvcdm {

class CryptoKey;
class UsageTableHeader;

typedef std::map<std::string, CryptoKey*> CryptoKeyMap;

// Crypto session utility functions used by KeySession implementations.
void GenerateMacContext(const std::string& input_context,
                        std::string* deriv_context);
void GenerateEncryptContext(const std::string& input_context,
                            std::string* deriv_context);
size_t GetOffset(std::string message, std::string field);
OEMCryptoCipherMode ToOEMCryptoCipherMode(CdmCipherMode cipher_mode);

class CryptoSession {
 public:
  typedef OEMCrypto_HDCP_Capability HdcpCapability;
  typedef enum {
    kUsageDurationsInvalid = 0,
    kUsageDurationPlaybackNotBegun = 1,
    kUsageDurationsValid = 2,
  } UsageDurationStatus;

  struct SupportedCertificateTypes {
    bool rsa_2048_bit;
    bool rsa_3072_bit;
    bool rsa_cast;
  };

  // Creates an instance of CryptoSession with the given |crypto_metrics|.
  // |crypto_metrics| is owned by the caller, must NOT be null, and must
  // exist as long as the new CryptoSession exists.
  explicit CryptoSession(metrics::CryptoMetrics* crypto_metrics);
  virtual ~CryptoSession();

  virtual bool GetProvisioningToken(std::string* client_token);
  virtual CdmClientTokenType GetPreProvisionTokenType() {
    return pre_provision_token_type_;
  }
  virtual CdmSecurityLevel GetSecurityLevel();
  virtual bool GetInternalDeviceUniqueId(std::string* device_id);
  virtual bool GetExternalDeviceUniqueId(std::string* device_id);
  virtual bool GetApiVersion(uint32_t* version);
  virtual bool GetSystemId(uint32_t* system_id);
  virtual bool GetProvisioningId(std::string* provisioning_id);
  virtual uint8_t GetSecurityPatchLevel();

  virtual CdmResponseType Open() { return Open(kLevelDefault); }
  virtual CdmResponseType Open(SecurityLevel requested_security_level);
  virtual void Close();

  virtual bool IsOpen() { return open_; }
  virtual CryptoSessionId oec_session_id() { return oec_session_id_; }

  // Key request/response
  virtual bool GenerateRequestId(std::string* req_id_str);
  virtual bool PrepareRequest(const std::string& key_deriv_message,
                              bool is_provisioning, std::string* signature);
  virtual bool PrepareRenewalRequest(const std::string& message,
                                     std::string* signature);
  virtual CdmResponseType LoadKeys(
      const std::string& message, const std::string& signature,
      const std::string& mac_key_iv, const std::string& mac_key,
      const std::vector<CryptoKey>& key_array,
      const std::string& provider_session_token,
      const std::string& srm_requirement,
      CdmLicenseKeyType key_type);
  virtual CdmResponseType LoadEntitledContentKeys(
      const std::vector<CryptoKey>& key_array);
  virtual bool LoadCertificatePrivateKey(std::string& wrapped_key);
  virtual bool RefreshKeys(const std::string& message,
                           const std::string& signature, int num_keys,
                           const CryptoKey* key_array);
  virtual bool GenerateNonce(uint32_t* nonce);
  virtual bool GenerateDerivedKeys(const std::string& message);
  virtual bool GenerateDerivedKeys(const std::string& message,
                                   const std::string& session_key);
  virtual bool RewrapCertificate(const std::string& signed_message,
                                 const std::string& signature,
                                 const std::string& nonce,
                                 const std::string& private_key,
                                 const std::string& iv,
                                 const std::string& wrapping_key,
                                 std::string* wrapped_private_key);

  // Media data path
  virtual CdmResponseType Decrypt(const CdmDecryptionParameters& params);

  // Usage related methods
  virtual bool UsageInformationSupport(bool* has_support);
  virtual CdmResponseType UpdateUsageInformation();  // only for OEMCrypto v9-12
  virtual CdmResponseType DeactivateUsageInformation(
      const std::string& provider_session_token);
  virtual CdmResponseType GenerateUsageReport(
      const std::string& provider_session_token, std::string* usage_report,
      UsageDurationStatus* usage_duration_status,
      int64_t* seconds_since_started, int64_t* seconds_since_last_played);
  virtual CdmResponseType ReleaseUsageInformation(
      const std::string& message, const std::string& signature,
      const std::string& provider_session_token);
  // Delete a usage information for a single token.  This does not require
  // a signed message from the server.
  virtual CdmResponseType DeleteUsageInformation(
      const std::string& provider_session_token);
  // Delete usage information for a list of tokens.  This does not require
  // a signed message from the server.
  virtual CdmResponseType DeleteMultipleUsageInformation(
      const std::vector<std::string>& provider_session_tokens);
  virtual CdmResponseType DeleteAllUsageReports();
  virtual bool IsAntiRollbackHwPresent();

  virtual bool GetHdcpCapabilities(HdcpCapability* current,
                                   HdcpCapability* max);
  virtual bool GetSupportedCertificateTypes(SupportedCertificateTypes* support);
  virtual bool GetRandom(size_t data_length, uint8_t* random_data);
  virtual bool GetNumberOfOpenSessions(size_t* count);
  virtual bool GetMaxNumberOfSessions(size_t* max);

  virtual bool GetSrmVersion(uint16_t* srm_version);
  virtual bool IsSrmUpdateSupported();
  virtual bool LoadSrm(const std::string& srm);

  virtual CdmResponseType GenericEncrypt(const std::string& in_buffer,
                                         const std::string& key_id,
                                         const std::string& iv,
                                         CdmEncryptionAlgorithm algorithm,
                                         std::string* out_buffer);
  virtual CdmResponseType GenericDecrypt(const std::string& in_buffer,
                                         const std::string& key_id,
                                         const std::string& iv,
                                         CdmEncryptionAlgorithm algorithm,
                                         std::string* out_buffer);
  virtual CdmResponseType GenericSign(const std::string& message,
                                      const std::string& key_id,
                                      CdmSigningAlgorithm algorithm,
                                      std::string* signature);
  virtual CdmResponseType GenericVerify(const std::string& message,
                                        const std::string& key_id,
                                        CdmSigningAlgorithm algorithm,
                                        const std::string& signature);

  // Usage table header and usage entry related methods
  virtual UsageTableHeader* GetUsageTableHeader() {
    return usage_table_header_;
  }
  virtual CdmResponseType GetUsageSupportType(CdmUsageSupportType* type);
  virtual CdmResponseType CreateUsageTableHeader(
      CdmUsageTableHeader* usage_table_header);
  virtual CdmResponseType LoadUsageTableHeader(
      const CdmUsageTableHeader& usage_table_header);
  virtual CdmResponseType CreateUsageEntry(uint32_t* entry_number);
  virtual CdmResponseType LoadUsageEntry(uint32_t entry_number,
                                         const CdmUsageEntry& usage_entry);
  virtual CdmResponseType UpdateUsageEntry(
      CdmUsageTableHeader* usage_table_header, CdmUsageEntry* usage_entry);
  virtual CdmResponseType ShrinkUsageTableHeader(
      uint32_t new_entry_count, CdmUsageTableHeader* usage_table_header);
  virtual CdmResponseType MoveUsageEntry(uint32_t new_entry_number);
  virtual bool CreateOldUsageEntry(uint64_t time_since_license_received,
                                   uint64_t time_since_first_decrypt,
                                   uint64_t time_since_last_decrypt,
                                   UsageDurationStatus status,
                                   const std::string& server_mac_key,
                                   const std::string& client_mac_key,
                                   const std::string& provider_session_token);
  virtual CdmResponseType CopyOldUsageEntry(
      const std::string& provider_session_token);
  virtual bool GetAnalogOutputCapabilities(bool* can_support_output,
                                           bool* can_disable_output,
                                           bool* can_support_cgms_a);
  virtual metrics::CryptoMetrics* GetCryptoMetrics() { return metrics_; }

  virtual CdmResponseType AddSubSession(const std::string& sub_session_key_id,
                                        const std::string& group_master_key_id);
  // TODO(jfore): exists is set based on whether a sub session exists. For now,
  // that is not assumed to be an error.
  virtual bool GenerateSubSessionNonce(const std::string& sub_session_key_id,
                                       bool* exists, uint32_t* nonce);

 private:
  friend class CryptoSessionForTest;

  CdmResponseType GetProvisioningMethod(SecurityLevel requested_security_level,
                                        CdmClientTokenType* token_type);
  void Init();
  void Terminate();
  bool GetTokenFromKeybox(std::string* token);
  bool GetTokenFromOemCert(std::string* token);
  static bool ExtractSystemIdFromOemCert(const std::string& oem_cert,
                                         uint32_t* system_id);
  bool GetSystemIdInternal(uint32_t* system_id);
  bool GenerateSignature(const std::string& message, std::string* signature);
  bool GenerateRsaSignature(const std::string& message, std::string* signature);

  bool SetDestinationBufferType();

  bool RewrapDeviceRSAKey(const std::string& message,
                          const std::string& signature,
                          const std::string& nonce,
                          const std::string& enc_rsa_key,
                          const std::string& rsa_key_iv,
                          std::string* wrapped_rsa_key);

  bool RewrapDeviceRSAKey30(const std::string& message,
                            const std::string& nonce,
                            const std::string& private_key,
                            const std::string& iv,
                            const std::string& wrapping_key,
                            std::string* wrapped_private_key);

  CdmResponseType SelectKey(const std::string& key_id,
                            CdmCipherMode cipher_mode);

  static const OEMCrypto_Algorithm kInvalidAlgorithm =
      static_cast<OEMCrypto_Algorithm>(-1);

  OEMCrypto_Algorithm GenericSigningAlgorithm(CdmSigningAlgorithm algorithm);
  OEMCrypto_Algorithm GenericEncryptionAlgorithm(
      CdmEncryptionAlgorithm algorithm);
  size_t GenericEncryptionBlockSize(CdmEncryptionAlgorithm algorithm);

  // These methods are used when a subsample exceeds the maximum buffer size
  // that the device can handle.
  OEMCryptoResult CopyBufferInChunks(
      const CdmDecryptionParameters& params,
      OEMCrypto_DestBufferDesc buffer_descriptor);
  OEMCryptoResult DecryptInChunks(
      const CdmDecryptionParameters& params,
      const OEMCrypto_DestBufferDesc& full_buffer_descriptor,
      const OEMCrypto_CENCEncryptPatternDesc& pattern_descriptor,
      size_t max_chunk_size);
  static void IncrementIV(uint64_t increase_by, std::vector<uint8_t>* iv_out);

  static const size_t kAes128BlockSize = 16;  // Block size for AES_CBC_128
  static const size_t kSignatureSize = 32;    // size for HMAC-SHA256 signature
  static Lock crypto_lock_;
  static bool initialized_;
  static int session_count_;

  metrics::CryptoMetrics* metrics_;
  metrics::TimerMetric life_span_;
  uint32_t system_id_;

  bool open_;
  CdmClientTokenType pre_provision_token_type_;
  std::string oem_token_;  // Cached OEMCrypto Public Key
  bool update_usage_table_after_close_session_;
  CryptoSessionId oec_session_id_;
  SubLicenseSessionMap sub_license_oec_sessions_;
  // Used for sub license sessions.
  std::string wrapped_key_;
  scoped_ptr<KeySession> key_session_;

  OEMCryptoBufferType destination_buffer_type_;
  bool is_destination_buffer_type_valid_;
  SecurityLevel requested_security_level_;

  bool is_usage_support_type_valid_;
  CdmUsageSupportType usage_support_type_;
  UsageTableHeader* usage_table_header_;
  static UsageTableHeader* usage_table_header_l1_;
  static UsageTableHeader* usage_table_header_l3_;

  uint64_t request_id_base_;
  static uint64_t request_id_index_;

  CdmCipherMode cipher_mode_;
  uint32_t api_version_;

  CORE_DISALLOW_COPY_AND_ASSIGN(CryptoSession);
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_CRYPTO_SESSSION_H_
