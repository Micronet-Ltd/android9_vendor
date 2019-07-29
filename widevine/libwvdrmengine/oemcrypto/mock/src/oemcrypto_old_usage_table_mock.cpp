// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
// This is from the v12 version of oemcrypto usage tables.  It is used for
// devices that upgrade from v12 to v13 in the field, and need to convert from
// the old type of usage table to the new.
#include "oemcrypto_old_usage_table_mock.h"

#include <string.h>
#include <time.h>

#include <string>
#include <vector>

#include <openssl/aes.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "file_store.h"
#include "log.h"
#include "oemcrypto_engine_mock.h"
#include "oemcrypto_logging.h"
#include "properties.h"
#include "pst_report.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

namespace wvoec_mock {

OldUsageTableEntry::OldUsageTableEntry(const std::vector<uint8_t> &pst_hash)
    : pst_hash_(pst_hash),
      time_of_license_received_(time(NULL)),
      time_of_first_decrypt_(0),
      time_of_last_decrypt_(0),
      status_(kUnused) {}

OldUsageTableEntry::~OldUsageTableEntry() {}

OldUsageTableEntry::OldUsageTableEntry(const OldStoredUsageEntry *buffer) {
  pst_hash_.assign(buffer->pst_hash, buffer->pst_hash + SHA256_DIGEST_LENGTH);
  time_of_license_received_ = buffer->time_of_license_received;
  time_of_first_decrypt_ = buffer->time_of_first_decrypt;
  time_of_last_decrypt_ = buffer->time_of_last_decrypt;
  status_ = buffer->status;
  mac_key_server_.assign(buffer->mac_key_server,
                         buffer->mac_key_server + wvcdm::MAC_KEY_SIZE);
  mac_key_client_.assign(buffer->mac_key_client,
                         buffer->mac_key_client + wvcdm::MAC_KEY_SIZE);
}

OldUsageTable::OldUsageTable(CryptoEngine *ce) {
  ce_ = ce;
  generation_ = 0;
  table_.clear();

  // Load saved table.
  wvcdm::FileSystem* file_system = ce->file_system();
  wvcdm::File *file;
  std::string path;
  // Note: this path is OK for a real implementation, but using security level 1
  // would be better.
  if (!wvcdm::Properties::GetDeviceFilesBasePath(wvcdm::kSecurityLevelL3,
                                                 &path)) {
    LOGE("OldUsageTable: Unable to get base path");
    return;
  }
  std::string filename = path + "UsageTable.dat";
  if (!file_system->Exists(filename)) {
    if (LogCategoryEnabled(kLoggingTraceUsageTable)) {
      LOGI("OldUsageTable: No saved usage table. Creating new table.");
    }
    return;
  }

  size_t file_size = file_system->FileSize(filename);
  std::vector<uint8_t> encrypted_buffer(file_size);
  std::vector<uint8_t> buffer(file_size);
  OldStoredUsageTable *stored_table =
      reinterpret_cast<OldStoredUsageTable *>(&buffer[0]);
  OldStoredUsageTable *encrypted_table =
      reinterpret_cast<OldStoredUsageTable *>(&encrypted_buffer[0]);

  file = file_system->Open(filename, wvcdm::FileSystem::kReadOnly);
  if (!file) {
    LOGE("OldUsageTable: File open failed: %s", path.c_str());
    return;
  }
  file->Read(reinterpret_cast<char *>(&encrypted_buffer[0]), file_size);
  file->Close();

  // Verify the signature of the usage table file.

  // This should be encrypted and signed with a device specific key.
  // For the reference implementation, I'm just going to use the keybox key.
  const bool override_to_real = true;
  const std::vector<uint8_t> &key = ce_->DeviceRootKey(override_to_real);

  uint8_t computed_signature[SHA256_DIGEST_LENGTH];
  unsigned int sig_length = sizeof(computed_signature);
  if (!HMAC(EVP_sha256(), &key[0], key.size(),
            &encrypted_buffer[SHA256_DIGEST_LENGTH],
            file_size - SHA256_DIGEST_LENGTH, computed_signature,
            &sig_length)) {
    LOGE("OldUsageTable: Could not recreate signature.");
    table_.clear();
    return;
  }
  if (memcmp(encrypted_table->signature, computed_signature, sig_length)) {
    LOGE("OldUsageTable: Invalid signature    given: %s",
         wvcdm::HexEncode(&encrypted_buffer[0], sig_length).c_str());
    LOGE("OldUsageTable: Invalid signature computed: %s",
         wvcdm::HexEncode(computed_signature, sig_length).c_str());
    table_.clear();
    return;
  }

  // Next, decrypt the table.
  uint8_t iv_buffer[wvcdm::KEY_IV_SIZE];
  memcpy(iv_buffer, encrypted_table->iv, wvcdm::KEY_IV_SIZE);
  AES_KEY aes_key;
  AES_set_decrypt_key(&key[0], 128, &aes_key);
  AES_cbc_encrypt(&encrypted_buffer[SHA256_DIGEST_LENGTH + wvcdm::KEY_IV_SIZE],
                  &buffer[SHA256_DIGEST_LENGTH + wvcdm::KEY_IV_SIZE],
                  file_size - SHA256_DIGEST_LENGTH - wvcdm::KEY_IV_SIZE,
                  &aes_key, iv_buffer, AES_DECRYPT);

  // Next, read the generation number from a different location.
  // On a real implementation, you should NOT put the generation number in
  // a file in user space.  It should be stored in secure memory. For the
  // reference implementation, we'll just pretend this is secure.
  std::string filename2 = path + "GenerationNumber.dat";
  file = file_system->Open(filename2, wvcdm::FileSystem::kReadOnly);
  if (!file) {
    LOGE("OldUsageTable: File open failed: %s (clearing table)", path.c_str());
    generation_ = 0;
    table_.clear();
    return;
  }
  file->Read(reinterpret_cast<char *>(&generation_), sizeof(int64_t));
  file->Close();
  if (stored_table->generation == generation_ + 1) {
    if (LogCategoryEnabled(kLoggingTraceUsageTable)) {
      LOGW("OldUsageTable: File is one generation old.  Acceptable rollback.");
    }
  } else if (stored_table->generation == generation_ - 1) {
    if (LogCategoryEnabled(kLoggingTraceUsageTable)) {
      LOGW("OldUsageTable: File is one generation new.  Acceptable rollback.");
    }
    // This might happen if the generation number was rolled back?
  } else if (stored_table->generation != generation_) {
    LOGE("OldUsageTable: Rollback detected. Clearing Usage Table. %lx -> %lx",
         generation_, stored_table->generation);
    table_.clear();
    generation_ = 0;
    return;
  }

  // At this point, the stored table looks valid. We can load in all the
  // entries.
  for (uint64_t i = 0; i < stored_table->count; i++) {
    OldUsageTableEntry *entry =
        new OldUsageTableEntry(&stored_table->entries[i].entry);
    table_[entry->pst_hash()] = entry;
  }
  if (LogCategoryEnabled(kLoggingTraceUsageTable)) {
    LOGI("OldUsageTable: loaded %d entries.", stored_table->count);
  }
}

OldUsageTableEntry *OldUsageTable::FindEntry(const std::vector<uint8_t> &pst) {
  wvcdm::AutoLock lock(lock_);
  return FindEntryLocked(pst);
}

OldUsageTableEntry *OldUsageTable::FindEntryLocked(const std::vector<uint8_t> &pst) {
  std::vector<uint8_t> pst_hash;
  if (!ComputeHash(pst, pst_hash)) {
    LOGE("OldUsageTable: Could not compute hash of pst.");
    return NULL;
  }
  EntryMap::iterator it = table_.find(pst_hash);
  if (it == table_.end()) {
    return NULL;
  }
  return it->second;
}

OldUsageTableEntry *OldUsageTable::CreateEntry(const std::vector<uint8_t> &pst) {
  std::vector<uint8_t> pst_hash;
  if (!ComputeHash(pst, pst_hash)) {
    LOGE("OldUsageTable: Could not compute hash of pst.");
    return NULL;
  }
  OldUsageTableEntry *entry = new OldUsageTableEntry(pst_hash);
  wvcdm::AutoLock lock(lock_);
  table_[pst_hash] = entry;
  return entry;
}

void OldUsageTable::Clear() {
  wvcdm::AutoLock lock(lock_);
  for (EntryMap::iterator i = table_.begin(); i != table_.end(); ++i) {
    if (i->second) delete i->second;
  }
  table_.clear();
}

void OldUsageTable::DeleteFile(CryptoEngine *ce) {
  wvcdm::FileSystem* file_system = ce->file_system();
  std::string path;
  // Note: this path is OK for a real implementation, but using security level 1
  // would be better.
  if (!wvcdm::Properties::GetDeviceFilesBasePath(wvcdm::kSecurityLevelL3,
                                                 &path)) {
    LOGE("OldUsageTable: Unable to get base path");
    return;
  }
  std::string filename = path + "UsageTable.dat";
  if (file_system->Exists(filename)) {
    if (!file_system->Remove(filename)) {
      LOGE("DeleteOldUsageTable: error removing file.");
    }
  }
}

bool OldUsageTable::ComputeHash(const std::vector<uint8_t> &pst,
                             std::vector<uint8_t> &pst_hash) {
  // The PST is not fixed size, and we have no promises that it is reasonbly
  // sized, so we compute a hash of it, and store that instead.
  pst_hash.resize(SHA256_DIGEST_LENGTH);
  SHA256_CTX context;
  if (!SHA256_Init(&context)) return false;
  if (!SHA256_Update(&context, &pst[0], pst.size())) return false;
  if (!SHA256_Final(&pst_hash[0], &context)) return false;
  return true;
}

}  // namespace wvoec_mock
