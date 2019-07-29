// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
//  This file contains oemcrypto engine properties that reads data from a file
//  to decide what it's current status is. It is used for testing cdm code.
//  The following properties are read from the file:
//  log_level: logging level to use.
//     0 = LOG_ERROR,
//     1 = LOG_WARN,
//     2 = LOG_INFO,
//     3 = LOG_DEBUG,
//     4 = LOG_VERBOSE
//  kLogging*: All logging flags found in oemcrypto/include/oemcrypto_logging.h
//     can be turned on (1) or off (0).
//  security_level: returned by OEMCrypto_SecurityLevel.
//  secure_lib: If set, then this will be used as a path to
//     the L1 liboemcrypto.so that we can use secure buffers.
//  current_hdcp: returned by OEMCrypto_GetHDCPCapability and
//     used to verify the key control block in methods like DecryptCENC.
//     HDCP_NONE  = 0,               // No HDCP supported, no secure data path.
//     HDCP_V1    = 1,               // HDCP version 1.0
//     HDCP_V2    = 2,               // HDCP version 2.0 Type 1.
//     HDCP_V2_1  = 3,               // HDCP version 2.1 Type 1.
//     HDCP_V2_2  = 4,               // HDCP version 2.2 Type 1.
//     HDCP_NO_DIGITAL_OUTPUT = 0xff // No digital output.
//  max_hdcp: returned by OEMCrypto_GetHDCPCapability.  Same values as above.
//  srm_update_supported: If "1", then srm update is supported.
//  srm_initial_version: Initial value for srm version.
//     This will be ignored after a reset.  If this is not set, CurrentSRM will
//     return NOT_IMPLEMENTED.
//  srm_load_fail: If set to a nonzero number, then load_srm will
//     fail and the version will not be updated.  The number is converted to
//     an OEMCryptoResult and returned.
//  srm_load_version: If this is set, then it will be used as the
//     new srm version after loading an SRM -- ignoring the contents of the SRM.
//     If srm_load_version is -1, then the buffer passed into LoadSRM will be
//     parsed.
//  srm_blacklisted_device_attached: If set to "1", then a
//     oemcrypto will act as if a blacklisted device is attached -- i.e.
//     playback will be restricted to the local display only.
//  srm_attached_device_id: If nonzero, the id of a blacklisted device.
//     If this id is in the revocation list of an SRM file when it is loaded,
//     playback will be restricted to the local display only.
//  security_patch_level: This is the value returned by
//     OEMCrypto_Security_Patch_Level.  If the key control block requires a
//     higher level, then OEMCrypto_LoadKeys will fail.
//  max_buffer_size: maximum size of a buffer accepted by DecryptCENC and
//     friends.  If this is 0, there is no restriction.  If it is 1, the
//     minimum allowed value is used.
//  use_keybox: If this is 1, then the test keybox is used.  If this is zero,
//     then the test OEM certificate is used.
//  use_fallback: If this is nonzero, then the installed Level 1 library will
//     be used to play content to a secure buffer.  Decryption and key
//     verification are done by the mock, but then the data is copied to the
//     secure buffer using OEMCrypto_CopyBuffer.  The filename of the fallback
//     library is hardcoded to "level1_backup_liboemcrypto.so".  It is
//     recommended you use the install script to ensure you have the right
//     filename.
//
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <string>
#include <map>

#include "log.h"
#include "oem_cert.h"
#include "oemcrypto_engine_mock.h"
#include "oemcrypto_logging.h"
#include "properties.h"
#include "string_conversions.h"

namespace wvoec_mock {
namespace {
typedef OEMCryptoResult (*L1_Initialize_t)(void);
typedef OEMCryptoResult (*L1_Terminate_t)(void);
typedef OEMCryptoResult (*L1_CopyBuffer_t)(const uint8_t* data_addr,
                                           size_t data_length,
                                           OEMCrypto_DestBufferDesc* out_buffer,
                                           uint8_t subsample_flags);
const std::string kDefaultOptionsFile = "/data/mediadrm/oemcrypto/options.txt";
}  // namespace

class AndroidModifiableCryptoEngine : public CryptoEngine {
 public:
  AndroidModifiableCryptoEngine(std::auto_ptr<wvcdm::FileSystem> file_system)
      : CryptoEngine(file_system),
        options_file_(kDefaultOptionsFile),
        srm_loaded_(false),
        srm_version_(0),
        level1_valid_(false),
        level1_library_(NULL) {
    std::string path;
    if (wvcdm::Properties::GetDeviceFilesBasePath(wvcdm::kSecurityLevelL3,
                                                  &path)) {
      options_file_ = path + "options.txt";
    }
  }

  void MaybeReadOptionsFile() {
    static time_t last_check = 0;
    static time_t last_changed = 0;
    time_t now = time(NULL);
    if (now > last_check + 5) {  // Check every five seconds.
      last_check = now;
      struct stat file_stat;
      if (stat(options_file_.c_str(), &file_stat)) {
        LOGE("Could not stat %s: %s", options_file_.c_str(), strerror(errno));
        return;
      }
      if (file_stat.st_mtime > last_changed) {
        last_changed = file_stat.st_mtime;
        ReadOptionsFile();
      }
    }
  }

  void ReadOptionsFile() {
    FILE *file = fopen(options_file_.c_str(), "r");
    if (!file) {
      LOGE("Could not read %s %s", options_file_.c_str(), strerror(errno));
      return;
    }
    while (!feof(file)) {
      char name[80 + 1];
      int64_t value;
      if (fscanf(file, "%80s %lld", name, &value)) {
        LOGD("Option %s = %lld", name, value);
        options_[std::string(name)] = value;
      }
    }
    fclose(file);
    InitializeLogging();
  }

  int64_t GetOption(const std::string &key, int64_t default_value) {
    MaybeReadOptionsFile();
    if (options_.find(key) == options_.end() ) {
      LOGW("Option %s not set.  Using default %lld", key.c_str(), default_value);
      return default_value;
    }
    return options_[key];
  }

  void InitializeLogging() {
    int log_level = GetOption("log_level", wvcdm::LOG_DEBUG);
    int categories = 0;
    if (GetOption("kLoggingTraceOEMCryptoCalls", 0) > 0)
      categories |= kLoggingTraceOEMCryptoCalls;
    if (GetOption("kLoggingDumpContentKeys", 0) > 0)
      categories |= kLoggingDumpContentKeys;
    if (GetOption("kLoggingDumpKeyControlBlocks", 0) > 0)
      categories |= kLoggingDumpKeyControlBlocks;
    if (GetOption("kLoggingDumpDerivedKeys", 0) > 0)
      categories |= kLoggingDumpDerivedKeys;
    if (GetOption("kLoggingTraceNonce", 0) > 0)
      categories |= kLoggingTraceNonce;
    if (GetOption("kLoggingTraceDecryption", 0) > 0)
      categories |= kLoggingTraceDecryption;
    if (GetOption("kLoggingTraceUsageTable", 0) > 0)
      categories |= kLoggingTraceUsageTable;
    if (GetOption("kLoggingTraceDecryptCalls", 0) > 0)
      categories |= kLoggingTraceDecryptCalls;
    if (GetOption("kLoggingDumpTraceAll", 0) > 0)
      categories |= kLoggingDumpTraceAll;
    SetLoggingSettings(log_level, categories);
  }

#define QUOTE_DEFINE(A) #A
#define QUOTE(A) QUOTE_DEFINE(A)
#define LOOKUP(Name, Function)                                          \
  Name = (L1_##Name##t)dlsym(level1_library_, QUOTE(Function));         \
  if (!Name) {                                                          \
    LOGW("Could not load L1 %s.",                                       \
         QUOTE(Function));                                              \
    Terminate();                                                        \
    return false;                                                       \
  }

  virtual bool Initialize() {
    LOGD("OEMCrypto Mock With Options "  " " __DATE__ " " __TIME__);
    MaybeReadOptionsFile();
    if (!GetOption("use_fallback", 1)) {
      LOGD("Level 1 fallback ignored.");
      return true;
    }
    level1_library_ = dlopen("level1_backup_liboemcrypto.so", RTLD_NOW);
    if (level1_library_ == NULL) {
      LOGE("Could not load backup: %s", dlerror());
      return false;
    }
    LOOKUP(Initialize_, OEMCrypto_Initialize);
    LOOKUP(Terminate_,  OEMCrypto_Terminate);
    LOOKUP(CopyBuffer_, OEMCrypto_CopyBuffer);
    level1_valid_ = true;
    OEMCryptoResult sts = Initialize_();
    LOGD("L1 fall back initialized.  status = %d.", sts);
    if (sts != OEMCrypto_SUCCESS) {
      LOGW("Terminating L1 because init failed.");
      Terminate();
      LOGW("Continuing Mock without L1 fallback.");
    }
    return true;
  }

  virtual void Terminate() {
    if (level1_valid_) Terminate_();
    if (level1_library_ != NULL) {
      LOGD("Closing L1 fall back.\n");
      dlclose(level1_library_);
      level1_valid_ = false;
      level1_library_ = NULL;
      CopyBuffer_ = NULL;
      Initialize_ = NULL;
      Terminate_ = NULL;
    } else {
      LOGD("Terminate mock.\n");
    }
  }

  const char *HDCPCapabilityAsString(OEMCrypto_HDCP_Capability value) {
    switch (value) {
      case HDCP_NONE:
        return "No HDCP supported, no secure data path";
      case HDCP_V1:
        return "HDCP version 1.0";
      case HDCP_V2:
        return "HDCP version 2.0";
      case HDCP_V2_1:
        return "HDCP version 2.1";
      case HDCP_V2_2:
        return "HDCP version 2.2";
      case HDCP_NO_DIGITAL_OUTPUT:
        return "No HDCP device attached/using local display with secure path";
      default:
        return "<INVALID VALUE>";
    }
  }


  OEMCrypto_ProvisioningMethod config_provisioning_method() {
    if (GetOption("use_keybox", 1)) {
      return OEMCrypto_Keybox;
    } else {
      return OEMCrypto_OEMCertificate;
    }
  }

  OEMCryptoResult get_oem_certificate(SessionContext* session,
                                      uint8_t* public_cert,
                                      size_t* public_cert_length) {
    if (GetOption("use_keybox", 1)) {
      LOGD("OEM Cert asked for when use_keybox = 1.");
      return OEMCrypto_ERROR_NOT_IMPLEMENTED;
    }
    if (kOEMPublicCertSize == 0) {
      LOGD("OEM Cert Size is 0.");
      return OEMCrypto_ERROR_NOT_IMPLEMENTED;
    }
    if (public_cert_length == NULL) {
      LOGD("OEM Cert length is 0.");
      return OEMCrypto_ERROR_UNKNOWN_FAILURE;
    }
    if (*public_cert_length < kOEMPublicCertSize) {
      *public_cert_length = kOEMPublicCertSize;
      return OEMCrypto_ERROR_SHORT_BUFFER;
    }
    *public_cert_length = kOEMPublicCertSize;
    if (public_cert == NULL) {
      return OEMCrypto_ERROR_SHORT_BUFFER;
    }
    memcpy(public_cert, kOEMPublicCert, kOEMPublicCertSize);
    if (!session->LoadRSAKey(kOEMPrivateKey, kOEMPrivateKeySize)) {
      LOGE("Private RSA Key did not load correctly.");
      return OEMCrypto_ERROR_INVALID_RSA_KEY;
    }
    return OEMCrypto_SUCCESS;
  }

  // Returns "L3" for a software only library.  L1 is for hardware protected
  // data paths.
  const char *config_security_level() {
    switch (GetOption("security_level", 0)) {
      default:
        LOGW("Option security_level not set.  Default is L3.");
      case 3:
        return "L3";
      case 2:
        return "L2";
      case 1:
        return "L1";
    }
  }

  // Returns the HDCP version currently in use.
  OEMCrypto_HDCP_Capability config_current_hdcp_capability() {
    static OEMCrypto_HDCP_Capability current_hdcp = HDCP_NONE;
    OEMCrypto_HDCP_Capability new_current_hdcp =
        static_cast<OEMCrypto_HDCP_Capability>(GetOption("current_hdcp", 0));
    if (current_hdcp != new_current_hdcp) {
      LOGI("OEMCrypto current HDCP changed from %d (%s) to %d (%s)", current_hdcp,
           HDCPCapabilityAsString(current_hdcp), new_current_hdcp,
           HDCPCapabilityAsString(new_current_hdcp));
      current_hdcp = new_current_hdcp;
    }
    return current_hdcp;
  }

  // Returns the max HDCP version supported.
  OEMCrypto_HDCP_Capability config_maximum_hdcp_capability() {
    static OEMCrypto_HDCP_Capability max_hdcp = HDCP_NONE;
    MaybeReadOptionsFile();
    OEMCrypto_HDCP_Capability new_max_hdcp =
        static_cast<OEMCrypto_HDCP_Capability>(GetOption("max_hdcp", 0));
    if (max_hdcp != new_max_hdcp) {
      LOGI("OEMCrypto max HDCP changed from %d (%s) to %d (%s)", max_hdcp,
           HDCPCapabilityAsString(max_hdcp), new_max_hdcp,
           HDCPCapabilityAsString(new_max_hdcp));
      max_hdcp = new_max_hdcp;
    }
    return max_hdcp;
  }

  // This should start at 0, and be incremented only when a security patch has
  // been applied to the device that fixes a security bug.
  uint8_t config_security_patch_level() {
    return GetOption("security_patch_level", 0);
  }

  size_t max_buffer_size() {
    int max = GetOption("max_buffer_size", 0);
    // If max is 1, just use default max buffer.
    if (max == 1) return CryptoEngine::max_buffer_size();
    return max;  // If 0, no restriction.  If something else, use that restriction.
  }

  bool srm_update_supported() {
    int supported = GetOption("srm_update_supported", 0);
    LOGI("OEMCrypto mock %s supporting SRM update.",
         supported ? "is" : "is not");
    return supported != 0;
  }

  OEMCryptoResult current_srm_version(uint16_t *version) {
    if (srm_loaded_) {
      LOGV("SRM loaded.  version used is %d.", srm_version_);
      *version = srm_version_;
      return OEMCrypto_SUCCESS;
    }
    int value = GetOption("srm_initial_version", -1);
    if (value > 0) {
      LOGV("SRM version from get option: %d.", value);
      srm_version_ = value;
      *version = value;
      return OEMCrypto_SUCCESS;
    } else {
      LOGI("SRM initial version is %d -- reporting not implemented.", value);
      return OEMCrypto_ERROR_NOT_IMPLEMENTED;
    }
  }

  // Convert uint24 or uint40 into a uint64.
  int64_t unpack_odd_bytes(const uint8_t *buffer, size_t length) {
    uint8_t small_buffer[8];
    memset(small_buffer, 0, 8);
    if (length > 8) {
      LOGE("OEMCrypto Mock: programmer error. unpack %d bytes.", length);
      length = 8;
    }
    size_t offset = 8 - length;
    memcpy(small_buffer + offset, buffer, length);
    return wvcdm::htonll64(*reinterpret_cast<const int64_t*>(small_buffer));
  }

  OEMCryptoResult load_srm(const uint8_t *buffer, size_t buffer_length) {
    if (!srm_update_supported()) {
      LOGE("OEMCrypto mock update not supported, but load_srm called.");
      return OEMCrypto_ERROR_NOT_IMPLEMENTED;
    }
    int result = GetOption("srm_load_fail", 0);
    if (result > 0) {
      LOGE("OEMCrypto mock load_srm returning error %d.", result);
      return static_cast<OEMCryptoResult>(result);
    }
    int new_version = GetOption("srm_load_version", -1);
    if (new_version >= 0) {
      if (new_version < srm_version_) {
        LOGE("New SRM version is lower than existing SRM version: %d < %d",
             new_version, srm_version_);
        return OEMCrypto_ERROR_INVALID_CONTEXT;
      }
      srm_version_ = new_version;
      LOGI("OEMCrypto mock told to change SRM version to %d.", srm_version_);
      srm_loaded_ = true;
      return OEMCrypto_SUCCESS;
    }
    if (buffer_length < 395) {
      LOGE("OEMCrypto mock bad buffer size: %ld < 395.", buffer_length);
      return OEMCrypto_ERROR_SHORT_BUFFER;
    }
    uint8_t srm_id = buffer[0] >> 4;
    uint8_t hdcp2_indicator = buffer[0] & 0x0F;
    uint8_t reserved = buffer[1];
    uint16_t version = htons(*reinterpret_cast<const uint16_t *>(&buffer[2]));
    if (reserved) {
      LOGE("OEMCrypto mock. SRM's second byte nonzero: %02X.", reserved);
      return OEMCrypto_ERROR_INVALID_CONTEXT;
    }
    uint8_t generation = buffer[4];
    if (generation > 1) {
      LOGW("OEMCrypto mock.  SRM Generation number is %d, but only first gen is parsed.",
           generation);
      LOGW("If the revoked device is in a a later generation, it will not be recognized.");
    }
    int64_t length = unpack_odd_bytes(buffer + 5, 3);  // 24 bits.
    if (length + 5 != buffer_length) {
      LOGW("OEMCrypto mock.  SRM length is %lld = 0x%llx, but I expected %zd = 0x%zx.",
           length, length, buffer_length - 5, buffer_length - 5);
    }
    int64_t count = 0;
    const uint8_t *ids;
    if (srm_id == 8 && hdcp2_indicator == 0) {
      // https://www.digital-cp.com/sites/default/files/specifications/HDCP%20Specification%20Rev1_4_Secure.pdf
      count = buffer[8];
      LOGI("OEMCrypto mock loading HDCP1 SRM.  version = %d. count=%lld.",
           version, count);
      ids = buffer + 9;
      if (buffer_length < 9 + count*5) {
        LOGE("OEMCrypto mock bad buffer size for count = %lld: %d < %lld.",
             count, buffer_length, 12 + count*5);
        return OEMCrypto_ERROR_SHORT_BUFFER;
      }
    } else if (srm_id == 9 && hdcp2_indicator == 1) {
      // https://www.digital-cp.com/sites/default/files/specifications/HDCP%20on%20HDMI%20Specification%20Rev2_2_Final1.pdf
      count =  unpack_odd_bytes(buffer + 8, 2) >> 6; // 10 bits = 2 bytes - 6.
      LOGI("OEMCrypto mock loading HDCP2 SRM.  version = %d. count=%lld.",
           version, count);
      ids = buffer + 12;
      if (buffer_length < 12 + count*5) {
        LOGE("OEMCrypto mock bad buffer size for count: %d < %ld.",
             buffer_length, 12 + count*5);
        return OEMCrypto_ERROR_SHORT_BUFFER;
      }
    } else {
      LOGE("OEMCrypto mock bad buffer start: %02X%02X%02X%02X...", buffer[0],
           buffer[1], buffer[2], buffer[3]);
      return OEMCrypto_ERROR_INVALID_CONTEXT;
    }
    for(size_t i = 0; i < count; i++) {
      int64_t id = unpack_odd_bytes(ids + 5*i, 5);
      srm_revocation_list_.push_back(id);
      LOGI("OEMCrypto mock SRM revokes device %lld = 0x%llx", id, id);
    }
    srm_loaded_ = true;
    srm_version_ = version;
    return OEMCrypto_SUCCESS;
  }

  OEMCryptoResult remove_srm() {
    if (!srm_update_supported()) {
      LOGE("OEMCrypto mock update not supported, bug load_srm called.");
      return OEMCrypto_ERROR_NOT_IMPLEMENTED;
    }
    srm_version_ = 0;
    srm_loaded_ = false;
    return OEMCrypto_SUCCESS;
  }

  bool srm_blacklisted_device_attached() {
    if (GetOption("srm_load_version", -1) < 0) {
      return scan_revoked_list();
    }
    static int blacklisted = 0;
    int new_value = GetOption("srm_blacklisted_device_attached", 0);
    if (new_value != blacklisted) {
      LOGI("SRM blacklisted device changed from %d to %d", blacklisted,
           new_value);
      blacklisted = new_value;
    }
    return blacklisted > 0;
  }

  bool scan_revoked_list() {
    static int64_t old_attached_id = 0;
    int64_t attached_id = GetOption("srm_attached_device_id", 0);
    bool print_all_ids = false;
    if (attached_id != old_attached_id) {
      LOGD("OEMCrypto mock -- ID of attached device is %lld = 0x%lld",
           attached_id, attached_id);
      old_attached_id = attached_id;
      print_all_ids = true;
    }
    for (size_t i = 0; i < srm_revocation_list_.size(); i++) {
      if (print_all_ids) {
        LOGD("OEMCrypto mock: %d) revoked id %lld = 0x%lld.", i,
             srm_revocation_list_[i], srm_revocation_list_[i]);
      }
      if (srm_revocation_list_[i] == attached_id) {
        LOGD("OEMCrypto mock: attached device %lld = 0x%lld is revoked.",
             attached_id, attached_id);
        return true;
      }
    }
    LOGD("OEMCrypto mock: attached device %lld is not revoked.", attached_id);
    return false;
  }

  virtual int nonce_flood_count() { GetOption("nonce_flood_count", 20); }

  virtual void adjust_destination(OEMCrypto_DestBufferDesc *out_description,
                                  size_t data_length, uint8_t subsample_flags) {
    if (out_description->type != OEMCrypto_BufferType_Secure) return;
    if (!level1_valid_) {
      static bool warned_once = false;
      if (!warned_once) {
        warned_once = true;
        LOGW("OEMCrypto Mock: given secure buffer with no level1 fallback.");
      }
      return;
    }
    if (subsample_flags & OEMCrypto_FirstSubsample) {
      final_destination_.type = OEMCrypto_BufferType_Secure;
      final_destination_.buffer.secure.handle =
          out_description->buffer.secure.handle;
      final_destination_.buffer.secure.max_length =
          out_description->buffer.secure.max_length;
      final_destination_.buffer.secure.offset =
          out_description->buffer.secure.offset;
      temp_buffer_.resize(final_destination_.buffer.secure.max_length);
      temp_buffer_length_ = 0;
    }
    if (temp_buffer_length_ != out_description->buffer.secure.offset) {
      LOGW("OEMCrypto: offset into secure buffer is not correct %zd != %zd.",
           temp_buffer_length_, out_description->buffer.secure.offset);
    }
    size_t new_length = temp_buffer_length_ + data_length;
    if (new_length > temp_buffer_.size()) {
      LOGW("Temp buffer was not big enough.  %zd > %zd.", new_length,
           temp_buffer_.size());
      temp_buffer_.resize(new_length);
    }
    destination_ = &temp_buffer_[temp_buffer_length_];
    temp_buffer_length_ = new_length;
  }

  // Push destination buffer to L1 output.
  virtual OEMCryptoResult PushDestination(
      OEMCrypto_DestBufferDesc *out_description, uint8_t subsample_flags) {
    if (level1_valid_ &&
        (out_description->type == OEMCrypto_BufferType_Secure)) {
      if (subsample_flags & OEMCrypto_LastSubsample) {
        return CopyBuffer_(&temp_buffer_[0], temp_buffer_length_,
                           &final_destination_,
                           OEMCrypto_FirstSubsample | OEMCrypto_LastSubsample);
      }
    }
    return OEMCrypto_SUCCESS;
  }

 private:
  // If the SRM version has been loaded or not.  If not, we use the system
  // property to find the current SRM version.
  bool srm_loaded_;
  // Current srm version.  Before an SRM has been loaded, this will be set from
  // the system property.
  int srm_version_;
  // List of forbidden/revoked devices.
  std::vector<int64_t> srm_revocation_list_;

  std::map<std::string, int64_t> options_;

  std::string options_file_;
  bool level1_valid_;
  void* level1_library_;
  L1_CopyBuffer_t CopyBuffer_;
  L1_Initialize_t Initialize_;
  L1_Terminate_t Terminate_;
  OEMCrypto_DestBufferDesc final_destination_;
  std::vector<uint8_t> temp_buffer_;
  size_t temp_buffer_length_;  // Length of temp buffer currently in use.
};

CryptoEngine* CryptoEngine::MakeCryptoEngine(
    std::auto_ptr<wvcdm::FileSystem> file_system) {
  return new AndroidModifiableCryptoEngine(file_system);
}

}  // namespace wvoec_mock
