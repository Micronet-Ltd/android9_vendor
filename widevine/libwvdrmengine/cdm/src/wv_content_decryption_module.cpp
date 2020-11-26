// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include "wv_content_decryption_module.h"

#include "cdm_client_property_set.h"
#include "cdm_engine.h"
#include "initialization_data.h"
#include "license.h"
#include "log.h"
#include "metrics.pb.h"
#include "properties.h"
#include "service_certificate.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_event_listener.h"

namespace {
const int kCdmPolicyTimerDurationSeconds = 1;
}

namespace wvcdm {

Lock WvContentDecryptionModule::session_sharing_id_generation_lock_;

WvContentDecryptionModule::WvContentDecryptionModule() {}

WvContentDecryptionModule::~WvContentDecryptionModule() {
  CloseAllCdms();
  DisablePolicyTimer();
}

bool WvContentDecryptionModule::IsSupported(const std::string& init_data_type) {
  return InitializationData(init_data_type).is_supported();
}

bool WvContentDecryptionModule::IsCenc(const std::string& init_data_type) {
  return InitializationData(init_data_type).is_cenc();
}

bool WvContentDecryptionModule::IsWebm(const std::string& init_data_type) {
  return InitializationData(init_data_type).is_webm();
}

bool WvContentDecryptionModule::IsHls(const std::string& init_data_type) {
  return InitializationData(init_data_type).is_hls();
}

CdmResponseType WvContentDecryptionModule::OpenSession(
    const CdmKeySystem& key_system, CdmClientPropertySet* property_set,
    const CdmIdentifier& identifier, WvCdmEventListener* event_listener,
    CdmSessionId* session_id) {
  if (property_set && property_set->is_session_sharing_enabled()) {
    AutoLock auto_lock(session_sharing_id_generation_lock_);
    if (property_set->session_sharing_id() == 0)
      property_set->set_session_sharing_id(GenerateSessionSharingId());
  }

  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->OpenSession(key_system, property_set,
                                                event_listener, session_id);
  cdm_engine->GetMetrics()->cdm_engine_open_session_.Increment(sts);
  if (sts == NO_ERROR) {
    cdm_by_session_id_[*session_id] = cdm_engine;
  }
  return sts;
}

CdmResponseType WvContentDecryptionModule::CloseSession(
    const CdmSessionId& session_id) {
  LOGV("WvContentDecryptionModule::CloseSession. id: %s", session_id.c_str());
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  // TODO(rfrias): Avoid reusing the error codes from CdmEngine.
  if (!cdm_engine) return SESSION_NOT_FOUND_1;
  AutoLock auto_lock(cdms_lock_);
  CdmResponseType sts = cdm_engine->CloseSession(session_id);
  cdm_engine->GetMetrics()->cdm_engine_close_session_.Increment(sts);
  if (sts == NO_ERROR) {
    cdm_by_session_id_.erase(session_id);
  }

  return sts;
}

bool WvContentDecryptionModule::IsOpenSession(const CdmSessionId& session_id) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  return cdm_engine && cdm_engine->IsOpenSession(session_id);
}

CdmResponseType WvContentDecryptionModule::GenerateKeyRequest(
    const CdmSessionId& session_id, const CdmKeySetId& key_set_id,
    const std::string& init_data_type, const CdmInitData& init_data,
    const CdmLicenseType license_type, CdmAppParameterMap& app_parameters,
    CdmClientPropertySet* property_set, const CdmIdentifier& identifier,
    CdmKeyRequest* key_request) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts;
  if (license_type == kLicenseTypeRelease) {
    sts = cdm_engine->OpenKeySetSession(key_set_id, property_set, NULL);
    cdm_engine->GetMetrics()->cdm_engine_open_key_set_session_.Increment(sts);
    if (sts != NO_ERROR) return sts;
    cdm_by_session_id_[key_set_id] = cdm_engine;
  }
  InitializationData initialization_data(init_data_type, init_data);
  M_TIME(sts = cdm_engine->GenerateKeyRequest(session_id, key_set_id,
                                              initialization_data, license_type,
                                              app_parameters, key_request),
         cdm_engine->GetMetrics(), cdm_engine_generate_key_request_, sts);
  switch (license_type) {
    case kLicenseTypeRelease:
      if (sts != KEY_MESSAGE) {
        cdm_engine->CloseKeySetSession(key_set_id);
        cdm_by_session_id_.erase(key_set_id);
      }
      break;
    default:
      if (sts == KEY_MESSAGE) EnablePolicyTimer();
      break;
  }
  return sts;
}

CdmResponseType WvContentDecryptionModule::AddKey(
    const CdmSessionId& session_id, const CdmKeyResponse& key_data,
    CdmKeySetId* key_set_id) {
  CdmEngine* cdm_engine = session_id.empty() ? GetCdmForSessionId(*key_set_id)
                                             : GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_3;
  // Save key_set_id, as CDM will return an empty key_set_id on release
  CdmKeySetId release_key_set_id;
  if (session_id.empty() && key_set_id != NULL) {
    release_key_set_id = *key_set_id;
  }
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->AddKey(session_id, key_data, key_set_id),
         cdm_engine->GetMetrics(), cdm_engine_add_key_, sts);
  if (sts == KEY_ADDED && session_id.empty()) {  // license type release
    cdm_engine->CloseKeySetSession(release_key_set_id);
    cdm_by_session_id_.erase(release_key_set_id);
  }
  return sts;
}

CdmResponseType WvContentDecryptionModule::RestoreKey(
    const CdmSessionId& session_id, const CdmKeySetId& key_set_id) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_4;
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->RestoreKey(session_id, key_set_id),
         cdm_engine->GetMetrics(), cdm_engine_restore_key_, sts);
  if (sts == KEY_ADDED) EnablePolicyTimer();
  return sts;
}

CdmResponseType WvContentDecryptionModule::RemoveKeys(
    const CdmSessionId& session_id) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_5;
  CdmResponseType sts = cdm_engine->RemoveKeys(session_id);
  cdm_engine->GetMetrics()->cdm_engine_remove_keys_.Increment(sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::QueryStatus(
    SecurityLevel security_level, const std::string& key, std::string* value) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(kDefaultCdmIdentifier);
  return cdm_engine->QueryStatus(security_level, key, value);
}

CdmResponseType WvContentDecryptionModule::QuerySessionStatus(
    const CdmSessionId& session_id, CdmQueryMap* key_info) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_8;
  return cdm_engine->QuerySessionStatus(session_id, key_info);
}

CdmResponseType WvContentDecryptionModule::QueryKeyStatus(
    const CdmSessionId& session_id, CdmQueryMap* key_info) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_9;
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->QueryKeyStatus(session_id, key_info),
         cdm_engine->GetMetrics(), cdm_engine_query_key_status_, sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::QueryOemCryptoSessionId(
    const CdmSessionId& session_id, CdmQueryMap* response) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return SESSION_NOT_FOUND_10;
  return cdm_engine->QueryOemCryptoSessionId(session_id, response);
}

CdmResponseType WvContentDecryptionModule::GetProvisioningRequest(
    CdmCertificateType cert_type, const std::string& cert_authority,
    const CdmIdentifier& identifier, const std::string& service_certificate,
    CdmProvisioningRequest* request, std::string* default_url) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->GetProvisioningRequest(cert_type, cert_authority,
                                                  service_certificate,
                                                  request, default_url),
         cdm_engine->GetMetrics(), cdm_engine_get_provisioning_request_, sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::HandleProvisioningResponse(
    const CdmIdentifier& identifier, CdmProvisioningResponse& response,
    std::string* cert, std::string* wrapped_key) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts;
  M_TIME(
      sts = cdm_engine->HandleProvisioningResponse(response, cert, wrapped_key),
      cdm_engine->GetMetrics(), cdm_engine_handle_provisioning_response_, sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::Unprovision(
    CdmSecurityLevel level, const CdmIdentifier& identifier) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->Unprovision(level);
  cdm_engine->GetMetrics()->cdm_engine_unprovision_.Increment(sts, level);
  return sts;
}

CdmResponseType WvContentDecryptionModule::GetUsageInfo(
    const std::string& app_id, const CdmIdentifier& identifier,
    CdmUsageInfo* usage_info) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->GetUsageInfo(app_id, usage_info),
         cdm_engine->GetMetrics(), cdm_engine_get_usage_info_, sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::GetUsageInfo(
    const std::string& app_id, const CdmSecureStopId& ssid,
    const CdmIdentifier& identifier, CdmUsageInfo* usage_info) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->GetUsageInfo(app_id, ssid, usage_info),
         cdm_engine->GetMetrics(), cdm_engine_get_usage_info_, sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::RemoveAllUsageInfo(
    const std::string& app_id, const CdmIdentifier& identifier) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->RemoveAllUsageInfo(app_id);
  cdm_engine->GetMetrics()->cdm_engine_remove_all_usage_info_.Increment(sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::RemoveUsageInfo(
    const std::string& app_id,
    const CdmIdentifier& identifier,
    const CdmSecureStopId& secure_stop_id) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->RemoveUsageInfo(app_id, secure_stop_id);
  cdm_engine->GetMetrics()->cdm_engine_remove_usage_info_.Increment(sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::ReleaseUsageInfo(
    const CdmUsageInfoReleaseMessage& message,
    const CdmIdentifier& identifier) {
  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->ReleaseUsageInfo(message);
  cdm_engine->GetMetrics()->cdm_engine_release_usage_info_.Increment(sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::GetSecureStopIds(
    const std::string& app_id,
    const CdmIdentifier& identifier,
    std::vector<CdmSecureStopId>* ssids) {
  if (ssids == NULL) {
    LOGE("WvContentDecryptionModule::GetSecureStopIds: ssid destination not "
         "provided");
    return PARAMETER_NULL;
  }

  CdmEngine* cdm_engine = EnsureCdmForIdentifier(identifier);
  CdmResponseType sts = cdm_engine->ListUsageIds(app_id, kSecurityLevelL1,
                                                 NULL, ssids);
  std::vector<CdmSecureStopId> secure_stop_ids;
  CdmResponseType sts_l3 = cdm_engine->ListUsageIds(app_id, kSecurityLevelL3,
                                                    NULL, &secure_stop_ids);
  ssids->insert(ssids->end(), secure_stop_ids.begin(), secure_stop_ids.end());
  if (sts_l3 != NO_ERROR) sts = sts_l3;
  cdm_engine->GetMetrics()->cdm_engine_get_secure_stop_ids_.Increment(sts);
  return sts;
}

CdmResponseType WvContentDecryptionModule::Decrypt(
    const CdmSessionId& session_id, bool validate_key_id,
    const CdmDecryptionParameters& parameters) {
  // First find the CdmEngine that has the given session_id.  If we are using
  // key sharing, the shared session will still be in the same CdmEngine.
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) {
    LOGE("WvContentDecryptionModule::Decrypt: session not found: %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_18;
  }

  CdmSessionId local_session_id = session_id;
  if (validate_key_id && Properties::GetSessionSharingId(session_id) != 0) {
    bool status =
        cdm_engine->FindSessionForKey(*parameters.key_id, &local_session_id);
    cdm_engine->GetMetrics()->cdm_engine_find_session_for_key_.Increment(
        status);
    if (!status && parameters.is_encrypted) {
      LOGE("WvContentDecryptionModule::Decrypt: unable to find session: %s",
           session_id.c_str());
      return KEY_NOT_FOUND_IN_SESSION;
    }
  }
  CdmResponseType sts;
  M_TIME(sts = cdm_engine->Decrypt(local_session_id, parameters),
         cdm_engine->GetMetrics(), cdm_engine_decrypt_, sts,
         metrics::Pow2Bucket(parameters.encrypt_length));
  return sts;
}

void WvContentDecryptionModule::NotifyResolution(const CdmSessionId& session_id,
                                                 uint32_t width,
                                                 uint32_t height) {
  CdmEngine* cdm_engine = GetCdmForSessionId(session_id);
  if (!cdm_engine) return;
  cdm_engine->NotifyResolution(session_id, width, height);
}

bool WvContentDecryptionModule::IsValidServiceCertificate(
    const std::string& certificate) {
  ServiceCertificate cert;
  CdmResponseType status = cert.Init(certificate);
  if (status != NO_ERROR) return false;
  return cert.has_certificate();
}


CdmResponseType WvContentDecryptionModule::GetMetrics(
    const CdmIdentifier& identifier, drm_metrics::WvCdmMetrics* metrics) {
  if (!metrics) {
    return PARAMETER_NULL;
  }
  AutoLock auto_lock(cdms_lock_);
  auto it = cdms_.find(identifier);
  if (it == cdms_.end()) {
    LOGE("WVContentDecryptionModule::GetMetrics. cdm_identifier not found");
    // TODO(blueeyes): Add a better error.
    return UNKNOWN_ERROR;
  }
  it->second.cdm_engine->GetMetrics()->Serialize(metrics);
  return NO_ERROR;
}

WvContentDecryptionModule::CdmInfo::CdmInfo()
    : cdm_engine(new CdmEngine(&file_system)) {}

CdmEngine* WvContentDecryptionModule::EnsureCdmForIdentifier(
    const CdmIdentifier& identifier) {
  AutoLock auto_lock(cdms_lock_);
  if (cdms_.find(identifier) == cdms_.end()) {
    // Accessing the map entry will create a new instance using the default
    // constructor. We then need to provide it with two pieces of info: The
    // origin provided by the app and an identifier that uniquely identifies
    // this CDM. We concatenate all pieces of the CdmIdentifier in order to
    // create an ID that is unique to that identifier.
    cdms_[identifier].file_system.SetOrigin(identifier.origin);
    cdms_[identifier].file_system.SetIdentifier(identifier.spoid +
                                                identifier.origin);

    // Set the app package name for use by metrics.
    cdms_[identifier].cdm_engine->GetMetrics()->SetAppPackageName(
        identifier.app_package_name);
  }
  CdmEngine* cdm_engine = cdms_[identifier].cdm_engine.get();

  return cdm_engine;
}

CdmEngine* WvContentDecryptionModule::GetCdmForSessionId(
    const std::string& session_id) {
  // Use find to avoid creating empty entries when not found.
  auto it = cdm_by_session_id_.find(session_id);
  if (it == cdm_by_session_id_.end()) return NULL;
  return it->second;
}

void WvContentDecryptionModule::CloseAllCdms() {
  AutoLock auto_lock(cdms_lock_);

  for (auto it = cdms_.begin(); it != cdms_.end();) {
    it = cdms_.erase(it);
  }
}

CdmResponseType WvContentDecryptionModule::CloseCdm(
    const CdmIdentifier& cdm_identifier) {
  // The policy timer ultimately calls OnTimerEvent (which wants to
  // acquire cdms_lock_). Therefore, we cannot acquire cdms_lock_ and then the
  // policy_timer_lock_ (via DisablePolicyTimer) at the same time.
  // Acquire the cdms_lock_ first, in its own scope.
  bool cdms_empty = false;
  {
    AutoLock auto_lock(cdms_lock_);
    auto it = cdms_.find(cdm_identifier);
    if (it == cdms_.end()) {
      LOGE("WVContentDecryptionModule::Close. cdm_identifier not found.");
      // TODO(blueeyes): Create a better error.
      return UNKNOWN_ERROR;
    }
    // Remove any sessions that point to this engine.
    for (auto session_it : cdm_by_session_id_) {
      if (session_it.second == it->second.cdm_engine.get()) {
        cdm_by_session_id_.erase(session_it.first);
      }
    }
    cdms_.erase(it);
    cdms_empty = cdms_.empty();
  }

  if (cdms_empty) {
    DisablePolicyTimer();
  }
  return NO_ERROR;
}

void WvContentDecryptionModule::EnablePolicyTimer() {
  AutoLock auto_lock(policy_timer_lock_);
  if (!policy_timer_.IsRunning())
    policy_timer_.Start(this, kCdmPolicyTimerDurationSeconds);
}

void WvContentDecryptionModule::DisablePolicyTimer() {
  AutoLock auto_lock(policy_timer_lock_);
  if (policy_timer_.IsRunning()) {
    policy_timer_.Stop();
  }
}

void WvContentDecryptionModule::OnTimerEvent() {
  AutoLock auto_lock(cdms_lock_);
  for (auto it = cdms_.begin(); it != cdms_.end(); ++it) {
    it->second.cdm_engine->OnTimerEvent();
  }
}

uint32_t WvContentDecryptionModule::GenerateSessionSharingId() {
  static int next_session_sharing_id = 0;
  return ++next_session_sharing_id;
}

}  // namespace wvcdm
