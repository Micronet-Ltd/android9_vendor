//
// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//

#ifndef WV_DRM_PLUGIN_H_
#define WV_DRM_PLUGIN_H_

#include <map>

#include <android/hardware/drm/1.1/IDrmPlugin.h>
#include <android/hardware/drm/1.0/IDrmPluginListener.h>
#include <android/hardware/drm/1.1/types.h>

#include "cdm_client_property_set.h"
#include "cdm_identifier.h"
#include "OEMCryptoCENC.h"
#include "wv_cdm_event_listener.h"
#include "wv_content_decryption_module.h"
#include "WVGenericCryptoInterface.h"
#include "WVTypes.h"

namespace wvdrm {
namespace hardware {
namespace drm {
namespace V1_1 {
namespace widevine {

using ::android::hardware::drm::V1_0::EventType;
using ::android::hardware::drm::V1_0::IDrmPluginListener;
using ::android::hardware::drm::V1_0::KeyRequestType;
using ::android::hardware::drm::V1_0::KeyStatus;
using ::android::hardware::drm::V1_0::KeyType;
using ::android::hardware::drm::V1_0::KeyValue;
using ::android::hardware::drm::V1_0::SecureStop;
using ::android::hardware::drm::V1_0::SecureStopId;
using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::drm::V1_1::DrmMetricGroup;
using ::android::hardware::drm::V1_1::HdcpLevel;
using ::android::hardware::drm::V1_1::IDrmPlugin;
using ::android::hardware::drm::V1_1::SecureStopRelease;
using ::android::hardware::drm::V1_1::SecurityLevel;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using std::map;
using wvcdm::CdmIdentifier;
using wvcdm::CdmKeyStatusMap;
using wvcdm::CdmSessionId;
using wvcdm::CdmResponseType;
using wvcdm::WvContentDecryptionModule;

const OEMCrypto_Algorithm kInvalidCryptoAlgorithm =
    static_cast<OEMCrypto_Algorithm>(-1);

struct WVDrmPlugin : public IDrmPlugin, IDrmPluginListener,
  wvcdm::WvCdmEventListener {

  WVDrmPlugin(const sp<WvContentDecryptionModule>& cdm,
              const std::string& appPackageName,
              WVGenericCryptoInterface* crypto,
              bool useSpoid);

  virtual ~WVDrmPlugin();

  Return<void> openSession(openSession_cb _hidl_cb) override;

  Return<void> openSession_1_1(SecurityLevel securityLevel, openSession_1_1_cb _hidl_cb) override;

  Return<Status> closeSession(const hidl_vec<uint8_t>& sessionId) override;

  Return<void> getKeyRequest(
      const hidl_vec<uint8_t>& scope,
      const hidl_vec<uint8_t>& initData,
      const hidl_string& mimeType,
      KeyType keyType,
      const hidl_vec<KeyValue>& optionalParameters,
      getKeyRequest_cb _hidl_cb) override;

  Return<void> getKeyRequest_1_1(
      const hidl_vec<uint8_t>& scope,
      const hidl_vec<uint8_t>& initData,
      const hidl_string& mimeType,
      KeyType keyType,
      const hidl_vec<KeyValue>& optionalParameters,
      getKeyRequest_1_1_cb _hidl_cb) override;

  Return<void> provideKeyResponse(
      const hidl_vec<uint8_t>& scope,
      const hidl_vec<uint8_t>& response,
      provideKeyResponse_cb _hidl_cb) override;

  Return<Status> removeKeys(const hidl_vec<uint8_t>& sessionId) override;

  Return<Status> restoreKeys(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<uint8_t>& keySetId) override;

  Return<void> queryKeyStatus(
      const hidl_vec<uint8_t>& sessionId,
      queryKeyStatus_cb _hidl_cb) override;

  Return<void> getProvisionRequest(
      const hidl_string& certificateType,
      const hidl_string& certificateAuthority,
      getProvisionRequest_cb _hidl_cb) override;

  Return<void> provideProvisionResponse(
      const hidl_vec<uint8_t>& response,
      provideProvisionResponse_cb _hidl_cb) override;

  Return<void> getSecureStops(getSecureStops_cb _hidl_cb) override;

  Return<void> getSecureStop(
      const hidl_vec<uint8_t>& secureStopId,
      getSecureStop_cb _hidl_cb) override;

  Return<Status> releaseAllSecureStops() override;

  Return<Status> releaseSecureStop(
      const hidl_vec<uint8_t>& secureStopId) override;

  Return<void> getMetrics(getMetrics_cb _hidl_cb);

  Return<void> getSecureStopIds(getSecureStopIds_cb _hidl_cb) override;

  Return<Status> releaseSecureStops(const SecureStopRelease& ssRelease) override;

  Return<Status> removeSecureStop(const hidl_vec<uint8_t>& secureStopId) override;

  Return<Status> removeAllSecureStops() override;

  Return<void> getHdcpLevels(getHdcpLevels_cb _hidl_cb) override;

  Return<void> getNumberOfSessions(getNumberOfSessions_cb _hidl_cb) override;

  Return<void> getSecurityLevel(const hidl_vec<uint8_t>& sessionId,
          getSecurityLevel_cb _hidl_cb) override;

  Return<void> getPropertyString(
      const hidl_string& propertyName,
      getPropertyString_cb _hidl_cb) override;

  Return<void> getPropertyByteArray(
      const hidl_string& propertyName,
      getPropertyByteArray_cb _hidl_cb) override;

  Return<Status> setPropertyString(
      const hidl_string& propertyName,
      const hidl_string& value) override;

  Return<Status> setPropertyByteArray(
      const hidl_string& propertyName,
      const hidl_vec<uint8_t>& value) override;

  Return<Status> setCipherAlgorithm(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_string& algorithm) override;

  Return<Status> setMacAlgorithm(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_string& algorithm) override;

  Return<void> encrypt(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<uint8_t>& keyId,
      const hidl_vec<uint8_t>& input,
      const hidl_vec<uint8_t>& iv,
      encrypt_cb _hidl_cb) override;

  Return<void> decrypt(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<uint8_t>& keyId,
      const hidl_vec<uint8_t>& input,
      const hidl_vec<uint8_t>& iv,
      decrypt_cb _hidl_cb) override;

  Return<void> sign(const hidl_vec<uint8_t>& sessionId,
       const hidl_vec<uint8_t>& keyId, const hidl_vec<uint8_t>& message,
       sign_cb _hidl_cb) override;

  Return<void> verify(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<uint8_t>& keyId,
      const hidl_vec<uint8_t>& message,
      const hidl_vec<uint8_t>& signature,
      verify_cb _hidl_cb) override;

  Return<void> signRSA(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_string& algorithm,
      const hidl_vec<uint8_t>& message,
      const hidl_vec<uint8_t>& wrappedkey,
      signRSA_cb _hidl_cb) override;

  Return<void> setListener(const sp<IDrmPluginListener>& listener) override;

  Return<void> sendEvent(
      EventType eventType,
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<uint8_t>& data) override;

  Return<void> sendExpirationUpdate(
      const hidl_vec<uint8_t>& sessionId,
      int64_t expiryTimeInMS) override;

  Return<void> sendKeysChange(
      const hidl_vec<uint8_t>& sessionId,
      const hidl_vec<KeyStatus>& keyStatusList,
      bool hasNewUsableKey) override;

  // The following methods do not use hidl interface, it is used internally.
  virtual Status unprovisionDevice();

  virtual void OnSessionRenewalNeeded(const CdmSessionId& cdmSessionId);

  virtual void OnSessionKeysChange(
      const CdmSessionId& cdmSessionId,
      const CdmKeyStatusMap& cdmKeysStatus,
      bool hasNewUsableKey);

  virtual void OnExpirationUpdate(
      const CdmSessionId& cdmSessionId,
      int64_t newExpiryTimeSeconds);

 private:
  WVDRM_DISALLOW_COPY_AND_ASSIGN_AND_NEW(WVDrmPlugin);

  struct CryptoSession {
   public:
    CryptoSession()
      : mOecSessionId(-1),
        mCipherAlgorithm(kInvalidCryptoAlgorithm),
        mMacAlgorithm(kInvalidCryptoAlgorithm) {}

    CryptoSession(OEMCrypto_SESSION sessionId)
      : mOecSessionId(sessionId),
        mCipherAlgorithm(kInvalidCryptoAlgorithm),
        mMacAlgorithm(kInvalidCryptoAlgorithm) {}

    OEMCrypto_SESSION oecSessionId() const { return mOecSessionId; }

    OEMCrypto_Algorithm cipherAlgorithm() const { return mCipherAlgorithm; }

    void setCipherAlgorithm(OEMCrypto_Algorithm newAlgorithm) {
      mCipherAlgorithm = newAlgorithm;
    }

    OEMCrypto_Algorithm macAlgorithm() const { return mMacAlgorithm; }

    void setMacAlgorithm(OEMCrypto_Algorithm newAlgorithm) {
      mMacAlgorithm = newAlgorithm;
    }

   private:
    OEMCrypto_SESSION mOecSessionId;
    OEMCrypto_Algorithm mCipherAlgorithm;
    OEMCrypto_Algorithm mMacAlgorithm;
  };

  class WVClientPropertySet : public wvcdm::CdmClientPropertySet {
   public:
    WVClientPropertySet()
      : mUsePrivacyMode(false), mShareKeys(false), mSessionSharingId(0) {}

    virtual ~WVClientPropertySet() {}

    virtual const std::string& security_level() const {
      return mSecurityLevel;
    }

    void set_security_level(const std::string& securityLevel) {
      mSecurityLevel = securityLevel;
    }

    virtual bool use_privacy_mode() const {
      return mUsePrivacyMode;
    }

    void set_use_privacy_mode(bool usePrivacyMode) {
      mUsePrivacyMode = usePrivacyMode;
    }

    virtual const std::string& service_certificate() const {
      return mServiceCertificate;
    }

    virtual void set_service_certificate(
        const std::string& serviceCertificate) {
      mServiceCertificate = serviceCertificate;
    }

    virtual const std::string& device_provisioning_service_certificate() const {
      // Android does not support service certificates for provisioning.
      return mEmptyString;
    }

    virtual void set_device_provisioning_service_certificate(
        const std::string& ) {
      // Ignore. Android does not support service certificates for provisioning
      // TODO(b/69562876): Android SHOULD support service cert for provisioning
    }

    virtual bool is_session_sharing_enabled() const {
      return mShareKeys;
    }

    void set_is_session_sharing_enabled(bool shareKeys) {
      mShareKeys = shareKeys;
    }

    virtual uint32_t session_sharing_id() const {
      return mSessionSharingId;
    }

    virtual void set_session_sharing_id(uint32_t id) {
      mSessionSharingId = id;
    }

    virtual const std::string& app_id() const {
      return mAppId;
    }

    void set_app_id(const std::string& appId) {
      mAppId = appId;
    }

   private:
    DISALLOW_EVIL_CONSTRUCTORS(WVClientPropertySet);

    std::string mSecurityLevel;
    bool mUsePrivacyMode;
    std::string mServiceCertificate;
    bool mShareKeys;
    uint32_t mSessionSharingId;
    std::string mAppId;
    const std::string mEmptyString;
  } mPropertySet;

  class CdmIdentifierBuilder {
   public:
    CdmIdentifierBuilder(bool useSpoid, const WVDrmPlugin& parent,
                         const std::string& appPackageName);

    // Fills in the passed-in struct with the CDM Identifier for the current
    // combination of Origin, Application, and Device. This is needed by some
    // calls into the CDM in order to identify which CDM instance should receive
    // the call. Calling this will seal the CDM Identifier Builder, thus making
    // it an error to change the origin.
    Status getCdmIdentifier(CdmIdentifier* identifier);

    // Gets the application-safe device-unique ID. On non-SPOID devices, this is
    // the device-unique ID from OEMCrypto. On SPOID devices, this is the SPOID.
    // On SPOID devices, calling this will seal the CDM Identifier Builder, thus
    // making it an error to change the origin.
    Status getDeviceUniqueId(std::string* id);
    Status getProvisioningUniqueId(std::string* id);

    const std::string& origin() const { return mCdmIdentifier.origin; }
    bool set_origin(const std::string& id);

    // Indicates whether the builder can still be modified. This returns false
    // until a call to getCdmIdentifier.
    bool is_sealed() { return mIsIdentifierSealed; }

   private:
    WVDRM_DISALLOW_COPY_AND_ASSIGN(CdmIdentifierBuilder);

    CdmIdentifier mCdmIdentifier;
    bool mIsIdentifierSealed;

    bool mUseSpoid;
    std::string mAppPackageName;
    const WVDrmPlugin& mParent;

    Status calculateSpoid();

    // Gets the device-unique ID from OEMCrypto. This must be private, since
    // this value must not be exposed to applications on SPOID devices. Code
    // outside this class should use getDeviceUniqueId() to get the
    // application-safe device-unique ID.
    Status getOemcryptoDeviceId(std::string* id);

    // The unique identifier is meant to ensure that two clients with the
    // same spoid, origin and app package name still get different cdm engine
    // instances. This is a stepping stone to simplifying the implementation.
    // Note that we do not have a lock or mutex around this object. We assume
    // that locking is handled external to this object.
    uint32_t getNextUniqueId();
  } mCdmIdentifierBuilder;

  sp<wvcdm::WvContentDecryptionModule> const mCDM;
  WVGenericCryptoInterface* mCrypto;
  map<CdmSessionId, CryptoSession> mCryptoSessions;
  sp<IDrmPluginListener> mListener;

  std::string mProvisioningServiceCertificate;

  Status queryProperty(const std::string& property,
                         std::string& stringValue) const;

  Status queryProperty(wvcdm::SecurityLevel securityLevel,
                         const std::string& property,
                         std::string& stringValue) const;

  Status queryProperty(const std::string& property,
                         std::vector<uint8_t>& vector_value) const;

  Status mapAndNotifyOfCdmResponseType(const std::vector<uint8_t>& sessionId,
                                         CdmResponseType res);

  Status mapAndNotifyOfOEMCryptoResult(const std::vector<uint8_t>& sessionId,
                                         OEMCryptoResult res);

  Status mapOEMCryptoResult(OEMCryptoResult res);

  SecurityLevel mapSecurityLevel(const std::string& level);

  Status openSessionCommon(std::vector<uint8_t>& sessionId);

  bool initDataResemblesPSSH(const std::vector<uint8_t>& initData);

  Status unprovision(const CdmIdentifier& identifier);
};

}  // namespace widevine
}  // namespace V1_1
}  // namespace drm
}  // namespace hardware
}  // namespace wvdrm

#endif // WV_DRM_PLUGIN_H_
