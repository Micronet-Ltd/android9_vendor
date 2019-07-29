// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#ifndef WVCDM_CORE_CDM_ENGINE_H_
#define WVCDM_CORE_CDM_ENGINE_H_

#include <memory>
#include <string>
#include <vector>

#include "cdm_session_map.h"
#include "certificate_provisioning.h"
#include "clock.h"
#include "crypto_session.h"
#include "file_store.h"
#include "initialization_data.h"
#include "lock.h"
#include "metrics_collections.h"
#include "oemcrypto_adapter.h"
#include "scoped_ptr.h"
#include "service_certificate.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_types.h"

namespace wvcdm {

class CdmClientPropertySet;
class CdmSession;
class CryptoEngine;
class UsagePropertySet;
class WvCdmEventListener;

// Keep expiration time for each key set
typedef std::map<CdmKeySetId,
                 std::pair<CdmSessionId, int64_t> > CdmReleaseKeySetMap;

class CdmEngine {
 public:
  CdmEngine(FileSystem* file_system, const std::string& spoid = EMPTY_SPOID);
  virtual ~CdmEngine();

  // Session related methods
  virtual CdmResponseType OpenSession(
      const CdmKeySystem& key_system, CdmClientPropertySet* property_set,
      const CdmSessionId& forced_session_id,
      WvCdmEventListener* event_listener);

  virtual CdmResponseType OpenSession(
      const CdmKeySystem& key_system, CdmClientPropertySet* property_set,
      WvCdmEventListener* event_listener, CdmSessionId* session_id);

  virtual CdmResponseType CloseSession(const CdmSessionId& session_id);

  virtual bool IsOpenSession(const CdmSessionId& session_id);

  virtual CdmResponseType OpenKeySetSession(
      const CdmKeySetId& key_set_id, CdmClientPropertySet* property_set,
      WvCdmEventListener* event_listener);

  virtual CdmResponseType CloseKeySetSession(const CdmKeySetId& key_set_id);

  // License related methods

  // Construct a valid license request. The arguments are used as follows:
  // session_id: The Session ID of the session the request is being generated
  //             for. This is ignored for license release requests.
  // key_set_id: The Key Set ID of the key set the request is being generated
  //             for. This is ignored except for license release requests.
  // init_data: The initialization data from the media file, which is used to
  //            build the key request. This is ignored for release and renewal
  //            requests.
  // license_type: The type of license being requested. Never ignored.
  // app_parameters: Additional, application-specific parameters that factor
  //                 into the request generation. This is ignored for release
  //                 and renewal requests.
  // key_request: This must be non-null and point to a CdmKeyRequest. The
  //              message field will be filled with the key request, the
  //              type field will be filled with the key request type,
  //              whether it is an initial request, renewal request,
  //              release request, etc. The url field will be filled with
  //              the default URL (if one is known) to send this key
  //              request to.
  virtual CdmResponseType GenerateKeyRequest(
      const CdmSessionId& session_id, const CdmKeySetId& key_set_id,
      const InitializationData& init_data, const CdmLicenseType license_type,
      CdmAppParameterMap& app_parameters, CdmKeyRequest* key_request);
  // This API may
  // (a) accept license response, extract key info and load keys.
  // (b) accept a renewal response and update license policy information.
  // (c) accept a release response and release an offline license or secure
  //     stop.
  // (d) accept a service certificate and cache that information for the
  //     the lifetime of the session.
  //
  // |session_id| identifies the session that generated the request and can
  //              process the response. Should be empty if a release response.
  // |key_data| is the license, renewal, release response or service
  //            certificate response.
  // |key_set_id| should be non-null and specified if license release.
  //              If offline license or streaming license associated with
  //              a secure stop, |key_set_id| should be non-null and will
  //              be filled in on return. Use the |key_set_id| with
  //              RestoreKeys (to reload offline session) or
  //              GenerateKeyRequest (to release offline session/secure stop).
  //              |key_set_id| will be cleared if release or streaming
  //              (not associated with a secure stop).
  virtual CdmResponseType AddKey(const CdmSessionId& session_id,
                                 const CdmKeyResponse& key_data,
                                 CdmKeySetId* key_set_id);

  virtual CdmResponseType RestoreKey(const CdmSessionId& session_id,
                                     const CdmKeySetId& key_set_id);

  virtual CdmResponseType RemoveKeys(const CdmSessionId& session_id);

  // Construct valid renewal request for the current session keys.
  virtual CdmResponseType GenerateRenewalRequest(
      const CdmSessionId& session_id, CdmKeyRequest* key_request);

  // Accept renewal response and update key info.
  virtual CdmResponseType RenewKey(const CdmSessionId& session_id,
                                   const CdmKeyResponse& key_data);

  // Query system information
  virtual CdmResponseType QueryStatus(SecurityLevel security_level,
                                      const std::string& query_token,
                                      std::string* query_response);

  // Query session information
  virtual CdmResponseType QuerySessionStatus(const CdmSessionId& session_id,
                                             CdmQueryMap* query_response);
  virtual bool IsReleaseSession(const CdmSessionId& session_id);
  virtual bool IsOfflineSession(const CdmSessionId& session_id);

  // Query license information
  virtual CdmResponseType QueryKeyStatus(const CdmSessionId& session_id,
                                         CdmQueryMap* query_response);

  // Query the types of usage permitted for the specified key.
  virtual CdmResponseType QueryKeyAllowedUsage(const CdmSessionId& session_id,
                                               const std::string& key_id,
                                               CdmKeyAllowedUsage* key_usage);

  // Query the types of usage permitted for the specified key.
  // Apply the query across all sessions.  If the key is found in more than
  // one session, return the allowed usage settings only if the usage settings
  // are identical for each instance of the key.  Otherwise, clear the settings
  // and return KEY_CONFLICT_1.
  virtual CdmResponseType QueryKeyAllowedUsage(const std::string& key_id,
                                               CdmKeyAllowedUsage* key_usage);

  // Query OEMCrypto session ID
  virtual CdmResponseType QueryOemCryptoSessionId(
      const CdmSessionId& session_id, CdmQueryMap* query_response);

  // Generate and return a valid provisioning request.
  virtual CdmResponseType GetProvisioningRequest(
      CdmCertificateType cert_type, const std::string& cert_authority,
      const std::string& service_certificate,
      CdmProvisioningRequest* request, std::string* default_url);

  // Verify and process a provisioning response.
  virtual CdmResponseType HandleProvisioningResponse(
      const CdmProvisioningResponse& response, std::string* cert,
      std::string* wrapped_key);

  // Return true if there is a device certificate on the current
  // (origin-specific) file system.
  virtual bool IsProvisioned(CdmSecurityLevel security_level);

  // Remove device DRM certificate from the current (origin-specific) file
  // system. This will force the device to reprovision itself.
  virtual CdmResponseType Unprovision(CdmSecurityLevel security_level);

  // Delete OEMCrypto usage tables. Used by Unprovision().
  virtual CdmResponseType DeleteUsageTable(CdmSecurityLevel security_level);

  // Return the list of key_set_ids stored on the current (origin-specific)
  // file system.
  virtual CdmResponseType ListStoredLicenses(
      CdmSecurityLevel security_level, std::vector<std::string>* key_set_ids);

  // Return the list of IDs associated with usage records for the
  // current (origin-specific) file system. At least one parameter
  // |ksids| or |provider_session_tokens| needs to be supplied.
  virtual CdmResponseType ListUsageIds(
      const std::string& app_id, CdmSecurityLevel security_level,
      std::vector<std::string>* ksids,
      std::vector<std::string>* provider_session_tokens);

  // Delete the usage record for the given key_set_id. This removes the
  // usage record in the file system and the OEMCrypto usage record.
  virtual CdmResponseType DeleteUsageRecord(const std::string& app_id,
                                            CdmSecurityLevel security_level,
                                            const std::string& key_set_id);

  // Usage related methods for streaming licenses
  // Retrieve a random usage info from the list of all usage infos for this app
  // id.
  virtual CdmResponseType GetUsageInfo(const std::string& app_id,
                                       CdmUsageInfo* usage_info);

  // Retrieve the usage info for the specified pst.
  // Returns UNKNOWN_ERROR if no usage info was found.
  virtual CdmResponseType GetUsageInfo(const std::string& app_id,
                                       const CdmSecureStopId& ssid,
                                       CdmUsageInfo* usage_info);

  // Remove all usage records for the current origin.
  virtual CdmResponseType RemoveAllUsageInfo(const std::string& app_id,
                                             CdmSecurityLevel security_level);

  // Remove all usage records for the current origin. Span all
  // security levels.
  virtual CdmResponseType RemoveAllUsageInfo(const std::string& app_id);

  virtual CdmResponseType RemoveUsageInfo(
      const std::string& app_id,
      const CdmSecureStopId& secure_stop_id);

  virtual CdmResponseType ReleaseUsageInfo(
      const CdmUsageInfoReleaseMessage& message);
  virtual CdmResponseType LoadUsageSession(const CdmKeySetId& key_set_id,
                                           CdmKeyMessage* release_message);

  // Decryption and key related methods
  // Accept encrypted buffer and return decrypted data.
  virtual CdmResponseType Decrypt(const CdmSessionId& session_id,
                                  const CdmDecryptionParameters& parameters);

  // Generic crypto operations - provides basic crypto operations that an
  // application can use outside of content stream processing

  // Encrypts a buffer of app-level data.
  virtual CdmResponseType GenericEncrypt(
      const std::string& session_id, const std::string& in_buffer,
      const std::string& key_id, const std::string& iv,
      CdmEncryptionAlgorithm algorithm, std::string* out_buffer);

  // Decrypts a buffer of app-level data.
  virtual CdmResponseType GenericDecrypt(
      const std::string& session_id, const std::string& in_buffer,
      const std::string& key_id, const std::string& iv,
      CdmEncryptionAlgorithm algorithm, std::string* out_buffer);

  // Computes the signature for a message.
  virtual CdmResponseType GenericSign(const std::string& session_id,
                                      const std::string& message,
                                      const std::string& key_id,
                                      CdmSigningAlgorithm algorithm,
                                      std::string* signature);

  // Verifies the signature on a buffer of app-level data.
  virtual CdmResponseType GenericVerify(const std::string& session_id,
                                        const std::string& message,
                                        const std::string& key_id,
                                        CdmSigningAlgorithm algorithm,
                                        const std::string& signature);

  virtual size_t SessionSize() const { return session_map_.Size(); }

  // Is the key known to any session?
  virtual bool IsKeyLoaded(const KeyId& key_id);
  virtual bool FindSessionForKey(const KeyId& key_id, CdmSessionId* sessionId);

  // Used for notifying the Max-Res Engine of resolution changes.
  // Return false if no match is found for session_id.
  virtual bool NotifyResolution(const CdmSessionId& session_id, uint32_t width,
                                uint32_t height);

  // Timer expiration method. This method is not re-entrant -- there can be
  // only one timer.
  // This method triggers appropriate event callbacks from |event_listener_|,
  // which is assumed to be asynchronous -- i.e. an event should be dispatched
  // to another thread which does the actual work. In particular, if a
  // synchronous listener calls OpenSession or CloseSession, the thread will
  // dead lock.
  virtual void OnTimerEvent();

  virtual metrics::EngineMetrics* GetMetrics() { return &metrics_; }

  virtual CdmResponseType ValidateServiceCertificate(const std::string& cert);

 private:
  // private methods
  CdmResponseType OpenSession(
      const CdmKeySystem& key_system, CdmClientPropertySet* property_set,
      WvCdmEventListener* event_listener, const CdmSessionId* forced_session_id,
      CdmSessionId* session_id);

  void DeleteAllUsageReportsUponFactoryReset();
  bool ValidateKeySystem(const CdmKeySystem& key_system);
  CdmResponseType GetUsageInfo(const std::string& app_id,
                               SecurityLevel requested_security_level,
                               CdmUsageInfo* usage_info);

  void OnKeyReleaseEvent(const CdmKeySetId& key_set_id);

  std::string MapHdcpVersion(CryptoSession::HdcpCapability version);

  void CloseExpiredReleaseSessions();

  // instance variables

  /*
   * The metrics group must be the first variable declared to ensure
   * that it is the last member destroyed so that no child members
   * try to use a reference to it after it is destroyed. This will
   * ensure that all data has been properly recorded in the group before
   * it is published.
   */
  metrics::EngineMetrics metrics_;

  CdmSessionMap session_map_;
  CdmReleaseKeySetMap release_key_sets_;
  scoped_ptr<CertificateProvisioning> cert_provisioning_;
  SecurityLevel cert_provisioning_requested_security_level_;
  FileSystem* file_system_;
  Clock clock_;
  std::string spoid_;

  static bool seeded_;

  // usage related variables
  scoped_ptr<CdmSession> usage_session_;
  scoped_ptr<UsagePropertySet> usage_property_set_;
  int64_t last_usage_information_update_time_;

  // Protect release_key_sets_ from non-thread-safe operations.
  Lock release_key_sets_lock_;

  // TODO(rfrias): Replace with two sets of locks, one to protect
  // the CdmSessionMap and a per-session lock to control access to
  // session usage/destruction.
  // Locks the session map |session_map_| and session usage/destruction
  // between session management calls (OpenSession, CloseSession, etc),
  // periodic timer calls (OnTimerEvent), and calls to Decrypt.
  // The layer above the CDM implementation is expected to handle thread
  // synchronization to make sure other functions that access sessions do not
  // occur simultaneously with OpenSession or CloseSession.
  Lock session_map_lock_;

  CORE_DISALLOW_COPY_AND_ASSIGN(CdmEngine);
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_CDM_ENGINE_H_
