// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include "usage_table_header.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "crypto_session.h"
#include "device_files.h"
#include "file_store.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_types.h"

namespace wvcdm {

namespace {

const std::string kEmptyString;

const CdmUsageTableHeader kEmptyUsageTableHeader;
const CdmUsageTableHeader kUsageTableHeader = "some usage table header";
const CdmUsageTableHeader kAnotherUsageTableHeader =
    "another usage table header";
const CdmUsageTableHeader kYetAnotherUsageTableHeader =
    "yet another usage table header";
const CdmUsageTableHeader kAndAnotherUsageTableHeader =
    "and another usage table header";
const CdmUsageTableHeader kOneMoreUsageTableHeader =
    "one more usage table header";
const CdmUsageEntry kUsageEntry = "usage entry";
const CdmUsageEntry kAnotherUsageEntry = "another usage entry";
const CdmUsageEntry kYetAnotherUsageEntry = "yet another usage entry";
const CdmUsageEntry kAndAnotherUsageEntry = "and another usage entry";
const CdmUsageEntry kOneMoreUsageEntry = "one more usage entry";
const CdmUsageEntryInfo kUsageEntryInfoOfflineLicense1 = {
    .storage_type = kStorageLicense,
    .key_set_id = "offline_key_set_1",
    .usage_info_file_name = ""};
const CdmUsageEntryInfo kUsageEntryInfoOfflineLicense2 = {
    .storage_type = kStorageLicense,
    .key_set_id = "offline_key_set_2",
    .usage_info_file_name = ""};
const CdmUsageEntryInfo kUsageEntryInfoOfflineLicense3 = {
    .storage_type = kStorageLicense,
    .key_set_id = "offline_key_set_3",
    .usage_info_file_name = ""};
const CdmUsageEntryInfo kUsageEntryInfoSecureStop1 = {
    .storage_type = kStorageUsageInfo,
    .key_set_id = "secure_stop_key_set_1",
    .usage_info_file_name = "usage_info_file_1"};
const CdmUsageEntryInfo kUsageEntryInfoSecureStop2 = {
    .storage_type = kStorageUsageInfo,
    .key_set_id = "secure_stop_key_set_2",
    .usage_info_file_name = "usage_info_file_2"};
const CdmUsageEntryInfo kUsageEntryInfoSecureStop3 = {
    .storage_type = kStorageUsageInfo,
    .key_set_id = "secure_stop_key_set_3",
    .usage_info_file_name = "usage_info_file_3"};
const CdmUsageEntryInfo kUsageEntryInfoStorageTypeUnknown = {
    .storage_type = kStorageTypeUnknown,
    .key_set_id = "",
    .usage_info_file_name = ""};
const CdmUsageEntryInfo kDummyUsageEntryInfo = {
    .storage_type = kStorageLicense,
    .key_set_id = "DummyKsid",
    .usage_info_file_name = ""};

const std::vector<std::string> kEmptyLicenseList;

const std::string kLicenseArray[] = {
    kUsageEntryInfoOfflineLicense1.key_set_id,
    kUsageEntryInfoOfflineLicense2.key_set_id,
    kUsageEntryInfoOfflineLicense3.key_set_id,
};
const size_t kLicenseArraySize = sizeof(kLicenseArray)/
    sizeof(kLicenseArray[0]);
std::vector<std::string> kLicenseList;

const std::vector<std::string> kEmptyUsageInfoFilesList;

const std::string kUsageInfoFileArray[] = {
    kUsageEntryInfoSecureStop1.usage_info_file_name,
    kUsageEntryInfoSecureStop2.usage_info_file_name,
    kUsageEntryInfoSecureStop3.usage_info_file_name,
};
const size_t kUsageInfoFileArraySize = sizeof(kUsageInfoFileArray)/
    sizeof(kUsageInfoFileArray[0]);
std::vector<std::string> kUsageInfoFileList;

const DeviceFiles::CdmUsageData kCdmUsageData1 = {
  .provider_session_token = "provider_session_token_1",
  .license_request = "license_request_1",
  .license = "license_1",
  .key_set_id = "key_set_id_1",
  .usage_entry = "usage_entry_1",
  .usage_entry_number = 0,
};
const DeviceFiles::CdmUsageData kCdmUsageData2 = {
  .provider_session_token = "provider_session_token_2",
  .license_request = "license_request_2",
  .license = "license_2",
  .key_set_id = "key_set_id_2",
  .usage_entry = "usage_entry_2",
  .usage_entry_number = 0,
};
const DeviceFiles::CdmUsageData kCdmUsageData3 = {
  .provider_session_token = "provider_session_token_3",
  .license_request = "license_request_3",
  .license = "license_3",
  .key_set_id = "key_set_id_3",
  .usage_entry = "usage_entry_3",
  .usage_entry_number = 0,
};
const std::vector<DeviceFiles::CdmUsageData> kEmptyUsageInfoUsageDataList;

const std::vector<CdmUsageEntryInfo> kEmptyUsageEntryInfoVector;
std::vector<CdmUsageEntryInfo> kUsageEntryInfoVector;
std::vector<CdmUsageEntryInfo> k10UsageEntryInfoVector;
std::vector<CdmUsageEntryInfo> k201UsageEntryInfoVector;

const DeviceFiles::LicenseState kActiveLicenseState =
    DeviceFiles::kLicenseStateActive;
const CdmInitData kPsshData = "pssh data";
const CdmKeyMessage kKeyRequest = "key request";
const CdmKeyResponse kKeyResponse = "key response";
const CdmKeyMessage kKeyRenewalRequest = "key renewal request";
const CdmKeyResponse kKeyRenewalResponse = "key renewal response";
const std::string kReleaseServerUrl = "some url";
const std::string kProviderSessionToken = "provider session token";
const CdmAppParameterMap kEmptyAppParameters;
int64_t kPlaybackStartTime = 1030005;
int64_t kPlaybackDuration = 300;
int64_t kGracePeriodEndTime = 60;

namespace {

void InitVectorConstants() {
  kUsageEntryInfoVector.clear();
  kUsageEntryInfoVector.push_back(kUsageEntryInfoOfflineLicense1);
  kUsageEntryInfoVector.push_back(kUsageEntryInfoSecureStop1);
  kUsageEntryInfoVector.push_back(kUsageEntryInfoStorageTypeUnknown);

  k10UsageEntryInfoVector.clear();
  k201UsageEntryInfoVector.clear();
  const CdmUsageEntryInfo* usage_entry_info =
      &kUsageEntryInfoStorageTypeUnknown;
  for (size_t i = 0; i < 201; ++i) {
    switch (i % 4) {
      case 0:
        usage_entry_info = &kUsageEntryInfoOfflineLicense1;
        break;
      case 1:
        usage_entry_info = &kUsageEntryInfoSecureStop1;
        break;
      case 2:
        usage_entry_info = &kUsageEntryInfoOfflineLicense2;
        break;
      case 3:
        usage_entry_info = &kUsageEntryInfoSecureStop2;
        break;
      default:
        usage_entry_info = &kUsageEntryInfoStorageTypeUnknown;
        break;
    }
    if (i < 10) {
      k10UsageEntryInfoVector.push_back(*usage_entry_info);
    }
    k201UsageEntryInfoVector.push_back(*usage_entry_info);
  }

  kUsageInfoFileList.clear();
  for (size_t i = 0; i < kUsageInfoFileArraySize; i++) {
    kUsageInfoFileList.push_back(kUsageInfoFileArray[i]);
  }

  kLicenseList.clear();
  for (size_t i = 0; i < kLicenseArraySize; i++) {
    kLicenseList.push_back(kLicenseArray[i]);
  }
}

void ToVector(std::vector<CdmUsageEntryInfo>& vec,
              const CdmUsageEntryInfo* arr, size_t total_size) {
  size_t max = total_size / sizeof(CdmUsageEntryInfo);
  vec.clear();
  for (size_t i = 0; i < max; i++) {
    vec.push_back(arr[i]);
  }
}

};  // namespace

class MockDeviceFiles : public DeviceFiles {
 public:
  MockDeviceFiles() : DeviceFiles(&file_system_) { Init(kSecurityLevelL1); }

  MOCK_METHOD2(RetrieveUsageTableInfo,
               bool(CdmUsageTableHeader*, std::vector<CdmUsageEntryInfo>*));
  MOCK_METHOD2(StoreUsageTableInfo,
               bool(const CdmUsageTableHeader&,
                    const std::vector<CdmUsageEntryInfo>&));
  MOCK_METHOD2(DeleteUsageInfo, bool(const std::string&, const std::string&));
  MOCK_METHOD7(RetrieveUsageInfoByKeySetId,
               bool(const std::string&, const std::string&, std::string*,
                    CdmKeyMessage*, CdmKeyResponse*, CdmUsageEntry*,
                    uint32_t*));
  MOCK_METHOD0(DeleteAllLicenses, bool());
  MOCK_METHOD0(DeleteAllUsageInfo, bool());
  MOCK_METHOD0(DeleteUsageTableInfo, bool());
  MOCK_METHOD7(StoreUsageInfo,
               bool(const std::string&, const CdmKeyMessage&,
                    const CdmKeyResponse&, const std::string&,
                    const std::string&, const CdmUsageEntry&, uint32_t));
  MOCK_METHOD2(RetrieveUsageInfo,
               bool(const std::string&, std::vector<CdmUsageData>*));
  MOCK_METHOD1(ListLicenses,
               bool(std::vector<std::string>* key_set_ids));
  MOCK_METHOD1(ListUsageInfoFiles,
               bool(std::vector<std::string>* usage_info_files));

 private:
  FileSystem file_system_;
};

class MockCryptoSession : public CryptoSession {
 public:
  MockCryptoSession(metrics::CryptoMetrics* metrics)
      : CryptoSession(metrics) {}
  MOCK_METHOD1(Open, CdmResponseType(SecurityLevel));
  MOCK_METHOD1(LoadUsageTableHeader,
               CdmResponseType(const CdmUsageTableHeader&));
  MOCK_METHOD1(CreateUsageTableHeader, CdmResponseType(CdmUsageTableHeader*));
  MOCK_METHOD1(CreateUsageEntry, CdmResponseType(uint32_t*));
  MOCK_METHOD2(LoadUsageEntry, CdmResponseType(uint32_t, const CdmUsageEntry&));
  MOCK_METHOD2(UpdateUsageEntry,
               CdmResponseType(CdmUsageTableHeader*, CdmUsageEntry*));
  MOCK_METHOD1(MoveUsageEntry, CdmResponseType(uint32_t));
  MOCK_METHOD2(ShrinkUsageTableHeader,
               CdmResponseType(uint32_t, CdmUsageTableHeader*));
};

}  // namespace

// gmock methods
using ::testing::_;
using ::testing::ElementsAreArray;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SizeIs;
using ::testing::StrEq;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

class UsageTableHeaderTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    InitVectorConstants();
  }

 protected:
  virtual void SetUp() {
    // UsageTableHeader will take ownership of the pointer
    device_files_ = new MockDeviceFiles();
    crypto_session_ = new MockCryptoSession(&crypto_metrics_);
    usage_table_header_ = new UsageTableHeader();

    // usage_table_header_ object takes ownership of these objects
    usage_table_header_->SetDeviceFiles(device_files_);
    usage_table_header_->SetCryptoSession(crypto_session_);
  }

  virtual void TearDown() {
    if (usage_table_header_ != NULL) delete usage_table_header_;
  }

  void Init(CdmSecurityLevel security_level,
            const CdmUsageTableHeader& usage_table_header,
            const std::vector<CdmUsageEntryInfo>& usage_entry_info_vector) {
    EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPointee<0>(usage_table_header),
                        SetArgPointee<1>(usage_entry_info_vector),
                        Return(true)));
    EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(usage_table_header))
        .WillOnce(Return(NO_ERROR));

    EXPECT_TRUE(usage_table_header_->Init(security_level, crypto_session_));
  }

  MockDeviceFiles* device_files_;
  metrics::CryptoMetrics crypto_metrics_;
  MockCryptoSession* crypto_session_;
  UsageTableHeader* usage_table_header_;
};

TEST_F(UsageTableHeaderTest, InitError) {
  EXPECT_FALSE(
      usage_table_header_->Init(kSecurityLevelUninitialized, crypto_session_));
  EXPECT_FALSE(usage_table_header_->Init(kSecurityLevelL2, crypto_session_));
  EXPECT_FALSE(
      usage_table_header_->Init(kSecurityLevelUnknown, crypto_session_));
  EXPECT_FALSE(usage_table_header_->Init(kSecurityLevelL1, NULL));
  EXPECT_FALSE(usage_table_header_->Init(kSecurityLevelL2, NULL));
}

class UsageTableHeaderInitializationTest
    : public UsageTableHeaderTest,
      public ::testing::WithParamInterface<CdmSecurityLevel> {
 public:
  static void SetUpTestCase() {
    InitVectorConstants();
  }

};

TEST_P(UsageTableHeaderInitializationTest, CreateUsageTableHeader) {
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyUsageTableHeader),
                      SetArgPointee<1>(kEmptyUsageEntryInfoVector),
                      Return(false)));
  EXPECT_CALL(*device_files_, ListLicenses(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyLicenseList),
                      Return(false)));
  EXPECT_CALL(*device_files_, ListUsageInfoFiles(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyUsageInfoFilesList),
                      Return(false)));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest, Upgrade_UnableToRetrieveLicenses) {
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyUsageTableHeader),
                      SetArgPointee<1>(kEmptyUsageEntryInfoVector),
                      Return(false)));
  EXPECT_CALL(*device_files_, ListLicenses(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kLicenseList),
                      Return(true)));
  EXPECT_CALL(*device_files_, ListUsageInfoFiles(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyUsageInfoFilesList),
                      Return(false)));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  // TODO: Why not being called?
  //EXPECT_CALL(*device_files_, DeleteAllLicenses()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .Times(2)
      .WillRepeatedly(Return(true));

  for (size_t i = 0; i < kLicenseList.size(); ++i)
    device_files_->DeleteLicense(kLicenseList[i]);
  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest, Upgrade_UnableToRetrieveUsageInfo) {
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyUsageTableHeader),
                      SetArgPointee<1>(kEmptyUsageEntryInfoVector),
                      Return(false)));
  EXPECT_CALL(*device_files_, ListLicenses(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kEmptyLicenseList),
                      Return(false)));
  EXPECT_CALL(*device_files_, ListUsageInfoFiles(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageInfoFileList),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .Times(2)
      .WillRepeatedly(Return(true));
  for (size_t i = 0; i < kUsageInfoFileList.size(); ++i) {
    EXPECT_CALL(*device_files_,
                RetrieveUsageInfo(kUsageInfoFileList[i], NotNull()))
      .WillOnce(DoAll(SetArgPointee<1>(kEmptyUsageInfoUsageDataList),
                      Return(false)));
  }

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest, UsageTableHeaderExists) {
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(kUsageEntryInfoVector), Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(NO_ERROR));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest, 200UsageEntries) {
  std::vector<CdmUsageEntryInfo> usage_entries_200 = k201UsageEntryInfoVector;
  usage_entries_200.resize(200);
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(usage_entries_200),
                          Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(NO_ERROR));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest,
       201UsageEntries_AddEntryFails_UsageTableHeaderRecreated) {
  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(k201UsageEntryInfoVector),
                          Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, DeleteAllLicenses()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, DeleteAllUsageInfo()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, DeleteUsageTableInfo()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .WillOnce(Return(true));

  // Expectations for AddEntry
  uint32_t expect_usage_entry_number = k201UsageEntryInfoVector.size();
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(expect_usage_entry_number),
                      Return(CREATE_USAGE_ENTRY_UNKNOWN_ERROR)));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest,
       201UsageEntries_DeleteEntryFails_UsageTableHeaderRecreated) {
  std::vector<CdmUsageEntryInfo> usage_entries_202 = k201UsageEntryInfoVector;
  usage_entries_202.push_back(kDummyUsageEntryInfo);

  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(k201UsageEntryInfoVector),
                          Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, DeleteAllLicenses()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, DeleteAllUsageInfo()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .WillOnce(Return(true));

  // Expectations for AddEntry
  uint32_t expect_usage_entry_number = k201UsageEntryInfoVector.size();
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(expect_usage_entry_number),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kUsageTableHeader,
                                                  usage_entries_202))
      .WillOnce(Return(true));

  // Expectations for DeleteEntry
  SecurityLevel security_level =
      (GetParam() == kSecurityLevelL3) ? kLevel3 : kLevelDefault;
  EXPECT_CALL(*crypto_session_,
              Open(security_level)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entries_202.size() - 1, NotNull()))
      .WillOnce(Return(SHRINK_USAGE_TABLER_HEADER_UNKNOWN_ERROR));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

TEST_P(UsageTableHeaderInitializationTest,
       201UsageEntries_AddDeleteEntrySucceeds) {
  std::vector<CdmUsageEntryInfo> usage_entries_202 = k201UsageEntryInfoVector;
  usage_entries_202.push_back(kDummyUsageEntryInfo);

  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(k201UsageEntryInfoVector),
                          Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(NO_ERROR));

  // Expectations for AddEntry
  uint32_t expect_usage_entry_number = k201UsageEntryInfoVector.size();
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(expect_usage_entry_number),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kUsageTableHeader,
                                                  usage_entries_202))
      .WillOnce(Return(true));

  // Expectations for DeleteEntry
  SecurityLevel security_level =
      (GetParam() == kSecurityLevelL3) ? kLevel3 : kLevelDefault;

  EXPECT_CALL(*crypto_session_,
              Open(security_level)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entries_202.size() - 1, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  SizeIs(k201UsageEntryInfoVector.size())))
      .WillOnce(Return(true));

  EXPECT_TRUE(usage_table_header_->Init(GetParam(), crypto_session_));
}

INSTANTIATE_TEST_CASE_P(Cdm, UsageTableHeaderInitializationTest,
                        ::testing::Values(kSecurityLevelL1, kSecurityLevelL3));

TEST_F(UsageTableHeaderTest, AddEntry_CreateUsageEntryFailed_UnknownError) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number;
  uint32_t expect_usage_entry_number = kUsageEntryInfoVector.size();

  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(expect_usage_entry_number),
                      Return(CREATE_USAGE_ENTRY_UNKNOWN_ERROR)));

  EXPECT_NE(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense1.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense1.key_set_id,
                kUsageEntryInfoOfflineLicense1.usage_info_file_name,
                &usage_entry_number));
}

TEST_F(UsageTableHeaderTest, AddEntry_UsageEntryTooSmall) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number;
  uint32_t expect_usage_entry_number = kUsageEntryInfoVector.size() - 1;

  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_NE(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense1.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense1.key_set_id,
                kUsageEntryInfoOfflineLicense1.usage_info_file_name,
                &usage_entry_number));
}

TEST_F(UsageTableHeaderTest, AddEntry_NextConsecutiveOfflineUsageEntry) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number;
  uint32_t expect_usage_entry_number = kUsageEntryInfoVector.size();
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      kUsageEntryInfoVector;

  expect_usage_entry_info_vector.resize(expect_usage_entry_number + 1);
  expect_usage_entry_info_vector[expect_usage_entry_number] =
      kUsageEntryInfoOfflineLicense2;

  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kUsageTableHeader,
                  UnorderedElementsAreArray(expect_usage_entry_info_vector)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense2.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense2.key_set_id,
                kUsageEntryInfoOfflineLicense2.usage_info_file_name,
                &usage_entry_number));
  EXPECT_EQ(expect_usage_entry_number, usage_entry_number);
}

TEST_F(UsageTableHeaderTest, AddEntry_NextConsecutiveSecureStopUsageEntry) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number;
  uint32_t expect_usage_entry_number = kUsageEntryInfoVector.size();
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      kUsageEntryInfoVector;

  expect_usage_entry_info_vector.resize(expect_usage_entry_number + 1);
  expect_usage_entry_info_vector[expect_usage_entry_number] =
      kUsageEntryInfoSecureStop2;

  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kUsageTableHeader,
                  UnorderedElementsAreArray(expect_usage_entry_info_vector)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoSecureStop2.storage_type == kStorageLicense,
                kUsageEntryInfoSecureStop2.key_set_id,
                kUsageEntryInfoSecureStop2.usage_info_file_name,
                &usage_entry_number));
  EXPECT_EQ(expect_usage_entry_number, usage_entry_number);
}

TEST_F(UsageTableHeaderTest, AddEntry_SkipUsageEntries) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number;
  uint32_t next_usage_entry_number = kUsageEntryInfoVector.size();
  size_t skip_usage_entries = 3;
  uint32_t expect_usage_entry_number =
      next_usage_entry_number + skip_usage_entries;

  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoSecureStop1,
              kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoSecureStop2)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoSecureStop2.storage_type == kStorageLicense,
                kUsageEntryInfoSecureStop2.key_set_id,
                kUsageEntryInfoSecureStop2.usage_info_file_name,
                &usage_entry_number));
  EXPECT_EQ(expect_usage_entry_number, usage_entry_number);
}

TEST_F(UsageTableHeaderTest,
       AddEntry_CreateUsageEntryFailsOnce_SucceedsSecondTime) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  uint32_t usage_entry_number_to_be_deleted = 0;
  CdmUsageEntryInfo usage_entry_to_be_deleted =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_deleted];
  uint32_t usage_entry_number_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;
  CdmUsageEntryInfo usage_entry_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_moved];

  // The last entry is moved to the entry to be deleted (0)
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_move[usage_entry_number_to_be_deleted] =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_moved];

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> shrunk_usage_entry_info_vector =
      usage_entry_info_vector_after_move;
  shrunk_usage_entry_info_vector.resize(
      shrunk_usage_entry_info_vector.size() - 1);

  // The new entry is then added to the end
  uint32_t expect_usage_entry_number = k10UsageEntryInfoVector.size() - 1;
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      shrunk_usage_entry_info_vector;
  expect_usage_entry_info_vector.push_back(kUsageEntryInfoOfflineLicense3);

  // Expectations for AddEntry
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(expect_usage_entry_info_vector)))
      .WillOnce(Return(true));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_to_be_moved.usage_info_file_name,
                     usage_entry_to_be_moved.key_set_id, kAnotherUsageEntry,
                     usage_entry_number_to_be_deleted))
      .WillOnce(Return(true));

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(shrunk_usage_entry_info_vector.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(shrunk_usage_entry_info_vector)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_to_be_moved.usage_info_file_name,
          usage_entry_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_to_be_moved), Return(true)));

  uint32_t usage_entry_number;
  EXPECT_EQ(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense3.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense3.key_set_id,
                kUsageEntryInfoOfflineLicense3.usage_info_file_name,
                &usage_entry_number));
  EXPECT_EQ(expect_usage_entry_number, usage_entry_number);
}

TEST_F(UsageTableHeaderTest,
       AddEntry_CreateUsageEntryFailsTwice_SucceedsThirdTime) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);

  // Initial usage entry info
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  // usage entry info after first move
  uint32_t usage_entry_number_first_to_be_deleted = 0;
  uint32_t usage_entry_number_first_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;

  // The last entry is moved to the first entry to be deleted
  CdmUsageEntryInfo usage_entry_first_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_first_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_first_move[
      usage_entry_number_first_to_be_deleted] =
          usage_entry_first_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_shrink =
      usage_entry_info_vector_after_first_move;
  usage_entry_info_vector_after_first_shrink.resize(
      usage_entry_info_vector_after_first_shrink.size() - 1);

  // usage entry info after second move
  uint32_t usage_entry_number_second_to_be_deleted = 1;
  uint32_t usage_entry_number_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink.size() - 1;

  // The last entry is moved to the second entry to be deleted
  CdmUsageEntryInfo usage_entry_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink[
          usage_entry_number_second_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_move =
      usage_entry_info_vector_after_first_shrink;
  usage_entry_info_vector_after_second_move[
      usage_entry_number_second_to_be_deleted] = usage_entry_second_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_shrink =
      usage_entry_info_vector_after_second_move;
  usage_entry_info_vector_after_second_shrink.resize(
      usage_entry_info_vector_after_second_shrink.size() - 1);

  // The new entry is then added to the end
  uint32_t expect_usage_entry_number =
      usage_entry_info_vector_after_second_shrink.size();
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      usage_entry_info_vector_after_second_shrink;
  expect_usage_entry_info_vector.push_back(kUsageEntryInfoOfflineLicense3);

  // Expectations for AddEntry
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(
          DoAll(SetArgPointee<0>(expect_usage_entry_number), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(expect_usage_entry_info_vector)))
      .WillOnce(Return(true));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_first_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_first_to_be_moved.usage_info_file_name,
                     usage_entry_first_to_be_moved.key_set_id,
                     kAnotherUsageEntry,
                     usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_second_to_be_moved.key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kYetAnotherUsageEntry,
      usage_entry_number_second_to_be_moved));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      usage_entry_second_to_be_moved.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kYetAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_second_to_be_moved, usage_entry_number);

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_first_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_second_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kYetAnotherUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_second_shrink)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(4)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_first_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_second_to_be_moved,
                             kYetAnotherUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_second_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kYetAnotherUsageTableHeader),
                      SetArgPointee<1>(kYetAnotherUsageEntry),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_second_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_first_to_be_moved.usage_info_file_name,
          usage_entry_first_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_first_to_be_moved),
          Return(true)));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense3.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense3.key_set_id,
                kUsageEntryInfoOfflineLicense3.usage_info_file_name,
                &usage_entry_number));
  EXPECT_EQ(expect_usage_entry_number, usage_entry_number);
}

TEST_F(UsageTableHeaderTest, AddEntry_CreateUsageEntryFailsThrice) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);

  // Initial usage entry info
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  // usage entry info after first move
  uint32_t usage_entry_number_first_to_be_deleted = 0;
  uint32_t usage_entry_number_first_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;

  // The last entry is moved to the first entry to be deleted
  CdmUsageEntryInfo usage_entry_first_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_first_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_first_move[
      usage_entry_number_first_to_be_deleted] =
          usage_entry_first_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_shrink =
      usage_entry_info_vector_after_first_move;
  usage_entry_info_vector_after_first_shrink.resize(
      usage_entry_info_vector_after_first_shrink.size() - 1);

  // usage entry info after second move
  uint32_t usage_entry_number_second_to_be_deleted = 1;
  uint32_t usage_entry_number_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink.size() - 1;

  // The last entry is moved to the second entry to be deleted
  CdmUsageEntryInfo usage_entry_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink[
          usage_entry_number_second_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_move =
      usage_entry_info_vector_after_first_shrink;
  usage_entry_info_vector_after_second_move[
      usage_entry_number_second_to_be_deleted] = usage_entry_second_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_shrink =
      usage_entry_info_vector_after_second_move;
  usage_entry_info_vector_after_second_shrink.resize(
      usage_entry_info_vector_after_second_shrink.size() - 1);

  // usage entry info after third move
  uint32_t usage_entry_number_third_to_be_deleted = 2;
  uint32_t usage_entry_number_third_to_be_moved =
      usage_entry_info_vector_after_second_shrink.size() - 1;

  // The last entry is moved to the third entry to be deleted
  CdmUsageEntryInfo usage_entry_third_to_be_moved =
      usage_entry_info_vector_after_second_shrink[
          usage_entry_number_third_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_third_move =
      usage_entry_info_vector_after_second_shrink;
  usage_entry_info_vector_after_third_move[
      usage_entry_number_third_to_be_deleted] = usage_entry_third_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_third_shrink =
      usage_entry_info_vector_after_third_move;
  usage_entry_info_vector_after_third_shrink.resize(
      usage_entry_info_vector_after_third_shrink.size() - 1);

  // Expected results after the third failure
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      usage_entry_info_vector_after_third_shrink;

  // Expectations for AddEntry
  EXPECT_CALL(*crypto_session_, CreateUsageEntry(NotNull()))
      .Times(3)
      .WillRepeatedly(Return(INSUFFICIENT_CRYPTO_RESOURCES_3));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_first_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_third_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_first_to_be_moved.usage_info_file_name,
                     usage_entry_first_to_be_moved.key_set_id,
                     kAnotherUsageEntry,
                     usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_third_to_be_moved.usage_info_file_name,
                     usage_entry_third_to_be_moved.key_set_id,
                     kOneMoreUsageEntry,
                     usage_entry_number_third_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_second_to_be_moved.key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kYetAnotherUsageEntry,
      usage_entry_number_second_to_be_moved));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      usage_entry_second_to_be_moved.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kYetAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_second_to_be_moved, usage_entry_number);

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_first_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_second_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kYetAnotherUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_third_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kOneMoreUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_second_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kOneMoreUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_third_shrink)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(6)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_first_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_second_to_be_moved,
                             kYetAnotherUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_third_to_be_moved,
                             kOneMoreUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_second_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_third_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kYetAnotherUsageTableHeader),
                      SetArgPointee<1>(kYetAnotherUsageEntry),
                      Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kOneMoreUsageTableHeader),
                      SetArgPointee<1>(kOneMoreUsageEntry),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_second_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kOneMoreUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_third_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_first_to_be_moved.usage_info_file_name,
          usage_entry_first_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_first_to_be_moved),
          Return(true)));
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_third_to_be_moved.usage_info_file_name,
          usage_entry_third_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kOneMoreUsageEntry),
          SetArgPointee<6>(usage_entry_number_third_to_be_moved),
          Return(true)));

  EXPECT_EQ(INSUFFICIENT_CRYPTO_RESOURCES_3,
            usage_table_header_->AddEntry(
                crypto_session_,
                kUsageEntryInfoOfflineLicense3.storage_type == kStorageLicense,
                kUsageEntryInfoOfflineLicense3.key_set_id,
                kUsageEntryInfoOfflineLicense3.usage_info_file_name,
                &usage_entry_number));
}

TEST_F(UsageTableHeaderTest, LoadEntry_InvalidEntryNumber) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number = kUsageEntryInfoVector.size() + 3;

  EXPECT_NE(NO_ERROR, usage_table_header_->LoadEntry(
                          crypto_session_, kUsageEntry, usage_entry_number));
}

TEST_F(UsageTableHeaderTest, LoadEntry_CryptoSessionError) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number = 1;

  EXPECT_CALL(*crypto_session_, LoadUsageEntry(usage_entry_number, kUsageEntry))
      .WillOnce(Return(LOAD_USAGE_ENTRY_GENERATION_SKEW));

  EXPECT_NE(NO_ERROR, usage_table_header_->LoadEntry(
                          crypto_session_, kUsageEntry, usage_entry_number));
}

TEST_F(UsageTableHeaderTest, LoadEntry) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number = 1;

  EXPECT_CALL(*crypto_session_, LoadUsageEntry(usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));

  EXPECT_EQ(NO_ERROR, usage_table_header_->LoadEntry(
                          crypto_session_, kUsageEntry, usage_entry_number));
}

TEST_F(UsageTableHeaderTest, UpdateEntry_CryptoSessionError) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  CdmUsageEntry usage_entry;

  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(kUsageEntry),
                      Return(UPDATE_USAGE_ENTRY_UNKNOWN_ERROR)));

  EXPECT_NE(NO_ERROR,
            usage_table_header_->UpdateEntry(crypto_session_, &usage_entry));
}

TEST_F(UsageTableHeaderTest, UpdateEntry) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  CdmUsageEntry usage_entry;

  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(kUsageEntry), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(kUsageTableHeader,
                          UnorderedElementsAreArray(kUsageEntryInfoVector)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->UpdateEntry(crypto_session_, &usage_entry));
}

TEST_F(UsageTableHeaderTest,
       LoadEntry_LoadUsageEntryFailsOnce_SucceedsSecondTime) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  uint32_t usage_entry_number_to_load = 5;

  uint32_t usage_entry_number_to_be_deleted = 0;
  CdmUsageEntryInfo usage_entry_to_be_deleted =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_deleted];
  uint32_t usage_entry_number_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;
  CdmUsageEntryInfo usage_entry_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_moved];

  // The last entry is moved to the entry to be deleted (0)
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_move[usage_entry_number_to_be_deleted] =
      usage_entry_info_vector_at_start[usage_entry_number_to_be_moved];

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> shrunk_usage_entry_info_vector =
      usage_entry_info_vector_after_move;
  shrunk_usage_entry_info_vector.resize(
      shrunk_usage_entry_info_vector.size() - 1);

  // Expectations for LoadEntry
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      shrunk_usage_entry_info_vector;

  EXPECT_CALL(*crypto_session_, LoadUsageEntry(usage_entry_number_to_load,
                                               kAndAnotherUsageEntry))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(Return(NO_ERROR));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_to_be_moved.usage_info_file_name,
                     usage_entry_to_be_moved.key_set_id, kAnotherUsageEntry,
                     usage_entry_number_to_be_deleted))
      .WillOnce(Return(true));

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(shrunk_usage_entry_info_vector.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(shrunk_usage_entry_info_vector)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_to_be_moved.usage_info_file_name,
          usage_entry_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_to_be_moved), Return(true)));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->LoadEntry(
                crypto_session_,
                kAndAnotherUsageEntry,
                usage_entry_number_to_load));
}

TEST_F(UsageTableHeaderTest,
       LoadEntry_LoadUsageEntryFailsTwice_SucceedsThirdTime) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);

  // Initial usage entry info
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  uint32_t usage_entry_number_to_load = 5;

  // usage entry info after first move
  uint32_t usage_entry_number_first_to_be_deleted = 0;
  uint32_t usage_entry_number_first_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;

  // The last entry is moved to the first entry to be deleted
  CdmUsageEntryInfo usage_entry_first_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_first_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_first_move[
      usage_entry_number_first_to_be_deleted] =
          usage_entry_first_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_shrink =
      usage_entry_info_vector_after_first_move;
  usage_entry_info_vector_after_first_shrink.resize(
      usage_entry_info_vector_after_first_shrink.size() - 1);

  // usage entry info after second move
  uint32_t usage_entry_number_second_to_be_deleted = 1;
  uint32_t usage_entry_number_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink.size() - 1;

  // The last entry is moved to the second entry to be deleted
  CdmUsageEntryInfo usage_entry_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink[
          usage_entry_number_second_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_move =
      usage_entry_info_vector_after_first_shrink;
  usage_entry_info_vector_after_second_move[
      usage_entry_number_second_to_be_deleted] = usage_entry_second_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_shrink =
      usage_entry_info_vector_after_second_move;
  usage_entry_info_vector_after_second_shrink.resize(
      usage_entry_info_vector_after_second_shrink.size() - 1);

  // Expectations for LoadEntry
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      usage_entry_info_vector_after_second_shrink;

  EXPECT_CALL(*crypto_session_, LoadUsageEntry(usage_entry_number_to_load,
                                               kAndAnotherUsageEntry))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(Return(INSUFFICIENT_CRYPTO_RESOURCES_3))
      .WillOnce(Return(NO_ERROR));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_first_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_first_to_be_moved.usage_info_file_name,
                     usage_entry_first_to_be_moved.key_set_id,
                     kAnotherUsageEntry,
                     usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_second_to_be_moved.key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kYetAnotherUsageEntry,
      usage_entry_number_second_to_be_moved));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      usage_entry_second_to_be_moved.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kYetAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_second_to_be_moved, usage_entry_number);

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_first_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_second_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kYetAnotherUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_second_shrink)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(4)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_first_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_second_to_be_moved,
                             kYetAnotherUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_second_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kYetAnotherUsageTableHeader),
                      SetArgPointee<1>(kYetAnotherUsageEntry),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_second_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_first_to_be_moved.usage_info_file_name,
          usage_entry_first_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_first_to_be_moved),
          Return(true)));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->LoadEntry(
                crypto_session_,
                kAndAnotherUsageEntry,
                usage_entry_number_to_load));
}

TEST_F(UsageTableHeaderTest, LoadEntry_LoadUsageEntryFailsThrice) {
  Init(kSecurityLevelL1, kUsageTableHeader, k10UsageEntryInfoVector);

  // Initial usage entry info
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_at_start =
      k10UsageEntryInfoVector;

  uint32_t usage_entry_number_to_load = 5;

  // usage entry info after first move
  uint32_t usage_entry_number_first_to_be_deleted = 0;
  uint32_t usage_entry_number_first_to_be_moved =
      usage_entry_info_vector_at_start.size() - 1;

  // The last entry is moved to the first entry to be deleted
  CdmUsageEntryInfo usage_entry_first_to_be_moved =
      usage_entry_info_vector_at_start[usage_entry_number_first_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_move =
      usage_entry_info_vector_at_start;
  usage_entry_info_vector_after_first_move[
      usage_entry_number_first_to_be_deleted] =
          usage_entry_first_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_first_shrink =
      usage_entry_info_vector_after_first_move;
  usage_entry_info_vector_after_first_shrink.resize(
      usage_entry_info_vector_after_first_shrink.size() - 1);

  // usage entry info after second move
  uint32_t usage_entry_number_second_to_be_deleted = 1;
  uint32_t usage_entry_number_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink.size() - 1;

  // The last entry is moved to the second entry to be deleted
  CdmUsageEntryInfo usage_entry_second_to_be_moved =
      usage_entry_info_vector_after_first_shrink[
          usage_entry_number_second_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_move =
      usage_entry_info_vector_after_first_shrink;
  usage_entry_info_vector_after_second_move[
      usage_entry_number_second_to_be_deleted] = usage_entry_second_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_second_shrink =
      usage_entry_info_vector_after_second_move;
  usage_entry_info_vector_after_second_shrink.resize(
      usage_entry_info_vector_after_second_shrink.size() - 1);

  // usage entry info after third move
  uint32_t usage_entry_number_third_to_be_deleted = 2;
  uint32_t usage_entry_number_third_to_be_moved =
      usage_entry_info_vector_after_second_shrink.size() - 1;

  // The last entry is moved to the third entry to be deleted
  CdmUsageEntryInfo usage_entry_third_to_be_moved =
      usage_entry_info_vector_after_second_shrink[
          usage_entry_number_third_to_be_moved];
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_third_move =
      usage_entry_info_vector_after_second_shrink;
  usage_entry_info_vector_after_third_move[
      usage_entry_number_third_to_be_deleted] = usage_entry_third_to_be_moved;

  // The entries are then shrunk by 1
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector_after_third_shrink =
      usage_entry_info_vector_after_third_move;
  usage_entry_info_vector_after_third_shrink.resize(
      usage_entry_info_vector_after_third_shrink.size() - 1);

  // Expectations for LoadEntry
  std::vector<CdmUsageEntryInfo> expect_usage_entry_info_vector =
      usage_entry_info_vector_after_third_shrink;

  EXPECT_CALL(*crypto_session_, LoadUsageEntry(usage_entry_number_to_load,
                                               kAndAnotherUsageEntry))
      .Times(3)
      .WillRepeatedly(Return(INSUFFICIENT_CRYPTO_RESOURCES_3));

  // Expectations for StoreEntry (DeleteEntry->MoveEntry)
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_first_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      DeleteUsageInfo(usage_entry_third_to_be_moved.usage_info_file_name,
                      kProviderSessionToken))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_first_to_be_moved.usage_info_file_name,
                     usage_entry_first_to_be_moved.key_set_id,
                     kAnotherUsageEntry,
                     usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     usage_entry_third_to_be_moved.usage_info_file_name,
                     usage_entry_third_to_be_moved.key_set_id,
                     kOneMoreUsageEntry,
                     usage_entry_number_third_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_second_to_be_moved.key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kYetAnotherUsageEntry,
      usage_entry_number_second_to_be_moved));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      usage_entry_second_to_be_moved.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kYetAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_second_to_be_moved, usage_entry_number);

  // Expectations for Shrink (DeleteEntry)
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_first_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_second_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kYetAnotherUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(
          usage_entry_info_vector_after_third_shrink.size(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kOneMoreUsageTableHeader),
                Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_second_shrink)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kOneMoreUsageTableHeader,
                  ElementsAreArray(
                      usage_entry_info_vector_after_third_shrink)))
      .WillOnce(Return(true));

  // Expectations for MoveEntry (DeleteEntry)
  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(6)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_first_to_be_moved,
                             kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_second_to_be_moved,
                             kYetAnotherUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(usage_entry_number_third_to_be_moved,
                             kOneMoreUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_first_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_second_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_third_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kYetAnotherUsageTableHeader),
                      SetArgPointee<1>(kYetAnotherUsageEntry),
                      Return(NO_ERROR)))
      .WillOnce(DoAll(SetArgPointee<0>(kOneMoreUsageTableHeader),
                      SetArgPointee<1>(kOneMoreUsageEntry),
                      Return(NO_ERROR)));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_first_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kYetAnotherUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_second_move)))
      .WillOnce(Return(true));
  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kOneMoreUsageTableHeader,
                  ElementsAreArray(usage_entry_info_vector_after_third_move)))
      .WillOnce(Return(true));

  // Expectations for GetEntry (DeleteEntry)
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_first_to_be_moved.usage_info_file_name,
          usage_entry_first_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_first_to_be_moved),
          Return(true)));
  EXPECT_CALL(*device_files_,
      RetrieveUsageInfoByKeySetId(
          usage_entry_third_to_be_moved.usage_info_file_name,
          usage_entry_third_to_be_moved.key_set_id, NotNull(), NotNull(),
          NotNull(), NotNull(), NotNull()))
      .Times(2)
      .WillRepeatedly(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kOneMoreUsageEntry),
          SetArgPointee<6>(usage_entry_number_third_to_be_moved),
          Return(true)));

  EXPECT_EQ(INSUFFICIENT_CRYPTO_RESOURCES_3,
            usage_table_header_->LoadEntry(
                crypto_session_,
                kAndAnotherUsageEntry,
                usage_entry_number_to_load));
}

TEST_F(UsageTableHeaderTest, DeleteEntry_InvalidUsageEntryNumber) {
  Init(kSecurityLevelL1, kUsageTableHeader, kUsageEntryInfoVector);
  uint32_t usage_entry_number = kUsageEntryInfoVector.size();
  metrics::CryptoMetrics metrics;

  EXPECT_NE(NO_ERROR, usage_table_header_->DeleteEntry(
                          usage_entry_number, device_files_, &metrics));
}

// Initial Test state:
// 1. Entry to be delete is the last entry and is an Offline license.
//    When attempting to delete the entry a crypto session error
//    will occur.
//
// Attempting to delete the entry in (1) will result in:
// a. The usage entry requested to be deleted will not be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Secure Stop 1                1          1
// Storage Type unknown         2          2
// Offline License 2            3          3
//
// # of usage entries           4          4
TEST_F(UsageTableHeaderTest, DeleteEntry_CryptoSessionError) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoSecureStop1,
      kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoOfflineLicense2};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoOfflineLicense2
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_info_vector.size() - 1, NotNull()))
      .WillOnce(Return(SHRINK_USAGE_TABLER_HEADER_UNKNOWN_ERROR));

  EXPECT_NE(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Entry to be delete is the last entry and is an Offline license.
//
// Attempting to delete the entry in (1) will result in:
// a. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Secure Stop 1                1          1
// Storage Type unknown         2          2
// Offline License 2            3    Deleted
//
// # of usage entries           4          3
TEST_F(UsageTableHeaderTest, DeleteEntry_LastOfflineEntry) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoSecureStop1,
      kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoOfflineLicense2};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoOfflineLicense2
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_info_vector.size() - 1, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoOfflineLicense1,
                                       kUsageEntryInfoSecureStop1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Entry to be delete is the last entry and is a secure stop.
//
// Attempting to delete the entry in (1) will result in:
// a. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Secure Stop 1                1          1
// Storage Type unknown         2          2
// Secure Stop 2                3    Deleted
//
// # of usage entries           4          3
TEST_F(UsageTableHeaderTest, DeleteEntry_LastSecureStopEntry) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
    const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoSecureStop1,
      kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoSecureStop2};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoSecureStop2
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_info_vector.size() - 1, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoOfflineLicense1,
                                       kUsageEntryInfoSecureStop1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Last few entries are offline licenses, but have license files
//    missing from persistent storage.
// 2. Usage entry to be deleted preceeds those in (1).
//
// Attempting to delete the entry in (2) will result in:
// a. Offline entries in (1) will be deleted.
// b. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2    Deleted
// Offline License 2            3    Deleted
// Offline License 3            4    Deleted
//
// # of usage entries           5          2
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastOfflineEntriesHaveMissingLicenses) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense1
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_number_to_be_deleted, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoSecureStop1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Last few entries are secure stops, but have entries
//    missing from usage info file in persistent storage.
// 2. Usage entry to be deleted preceeds those in (1).
//
// Attempting to delete the entry in (2) will result in:
// a. Secure stops in (1) will be deleted.
// b. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2    Deleted
// Secure stop 2                3    Deleted
// Secure stop 3                4    Deleted
//
// # of usage entries           5          2
TEST_F(UsageTableHeaderTest, DeleteEntry_LastSecureStopEntriesAreMissing) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1, kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoSecureStop1
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_number_to_be_deleted, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop2.usage_info_file_name,
                  kUsageEntryInfoSecureStop2.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(Return(false));
  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(Return(false));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoOfflineLicense1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Last few entries are offline licenses, but have incorrect usage
//    entry number stored in persistent file store.
// 2. Usage entry to be deleted preceeds those in (1).
//
// Attempting to delete the entry in (2) will result in:
// a. Offline entries in (1) will be deleted.
// b. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2    Deleted
// Offline License 2            3    Deleted
// Offline License 3            4    Deleted
//
// # of usage entries           5          2
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastOfflineEntriesHaveIncorrectUsageEntryNumber) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense1
  metrics::CryptoMetrics metrics;

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[usage_entry_info_vector.size() - 1].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      usage_entry_info_vector.size() - 2));

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[usage_entry_info_vector.size() - 2].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      usage_entry_info_vector.size() - 3));

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_number_to_be_deleted, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoSecureStop1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Last few entries are secure stops, but have incorrect usage
//    entry number stored in persistent file store.
// 2. Usage entry to be deleted preceeds those in (1).
//
// Attempting to delete the entry in (2) will result in:
// a. Secure stops entries in (1) will be deleted.
// b. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2    Deleted
// Secure stop 2                3    Deleted
// Secure stop 3                4    Deleted
//
// # of usage entries           5          2
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastSecureStopEntriesHaveIncorrectUsageEntryNumber) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1, kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoSecureStop1
  uint32_t usage_entry_number_after_deleted_entry =
      usage_entry_number_to_be_deleted + 1;
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_number_to_be_deleted, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop2.usage_info_file_name,
                  kUsageEntryInfoSecureStop2.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(DoAll(
          SetArgPointee<2>(kProviderSessionToken),
          SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
          SetArgPointee<5>(kUsageEntry),
          SetArgPointee<6>(usage_entry_number_to_be_deleted), Return(true)));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<2>(kProviderSessionToken),
                      SetArgPointee<3>(kKeyRequest),
                      SetArgPointee<4>(kKeyResponse),
                      SetArgPointee<5>(kUsageEntry),
                      SetArgPointee<6>(usage_entry_number_after_deleted_entry),
                      Return(true)));

  EXPECT_CALL(*device_files_,
              StoreUsageTableInfo(
                  kAnotherUsageTableHeader,
                  UnorderedElementsAre(kUsageEntryInfoOfflineLicense1,
                                       kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Last few entries are of storage type unknown.
// 2. Usage entry to be deleted preceeds those in (1).
//
// Attempting to delete the entry in (2) will result in:
// a. Entries of storage type unknown at the end will be deleted.
// b. The usage entry requested to be deleted will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2          2
// Offline License 2            3          3
// Offline License 3            4    Deleted
// Storage Type unknown         5    Deleted
// Storage Type unknown         6    Deleted
//
// # of usage entries           7          4
TEST_F(UsageTableHeaderTest, DeleteEntry_LastEntriesAreStorageTypeUnknown) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1,        kUsageEntryInfoOfflineLicense1,
      kUsageEntryInfoOfflineLicense2,    kUsageEntryInfoOfflineLicense3,
      kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoStorageTypeUnknown};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(usage_entry_number_to_be_deleted, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(kAnotherUsageTableHeader,
                          UnorderedElementsAre(kUsageEntryInfoSecureStop1,
                                               kUsageEntryInfoOfflineLicense1,
                                               kUsageEntryInfoOfflineLicense2)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last entry is an offline license and calling
//    OEMCrypto_MoveUsageEntry on it will fail.
//
// Attempting to delete the entry in (1) will result in:
// b. The last offline usage entry will not be deleted/moved if the
//    OEMCrypto_MoveUsageEntry operation fails.
// c. The usage entry requested to be deleted will be marked as
//     storage type unknown.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2  Deleted/storage type unknown
// Offline License 2            3          3
// Offline License 3            4          4
//
// # of usage entries           5          5
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastEntryIsOffline_MoveOfflineEntryFailed) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense1
  uint32_t last_usage_entry_number =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[last_usage_entry_number].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      last_usage_entry_number));

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(MOVE_USAGE_ENTRY_UNKNOWN_ERROR));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoOfflineLicense2,
              kUsageEntryInfoOfflineLicense3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last entry is an secure stop and calling
//    OEMCrypto_MoveUsageEntry on it will fail.
//
// Attempting to delete the entry in (1) will result in:
// b. The last secure stop usage entry will not be deleted/moved if the
//    OEMCrypto_MoveUsageEntry operation fails.
// c. The usage entry requested to be deleted will be marked as
//     storage type unknown.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2  Deleted/storage type unknown
// Secure stop 2                3          3
// Secure stop 3                4          4
//
// # of usage entries           5          5
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastEntryIsSecureStop_MoveSecureStopEntryFailed) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1, kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoSecureStop1
  uint32_t last_usage_entry_number =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoSecureStop3
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault)).WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(MOVE_USAGE_ENTRY_UNKNOWN_ERROR));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<2>(kProviderSessionToken),
                      SetArgPointee<3>(kKeyRequest),
                      SetArgPointee<4>(kKeyResponse),
                      SetArgPointee<5>(kUsageEntry),
                      SetArgPointee<6>(last_usage_entry_number), Return(true)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoSecureStop2,
              kUsageEntryInfoSecureStop3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last
// 2. Last few entries are of storage type unknown.
// 3. Entry that preceeds those in (2) is an offline license and calling
//    OEMCrypto_MoveUsageEntry on it will fail.
//
// Attempting to delete the entry in (1) will result in:
// a. Entries of storage type unknown at the end will be deleted.
// b. The offline usage entry that preceeds the entries in (a) will
//    not be deleted/moved if the OEMCrypto_MoveUsageEntry operation fails.
// c. The usage entry requested to be deleted will be marked as
//     storage type unknown.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2  Deleted/storage type unknown
// Offline License 2            3          3
// Offline License 3            4          4
// Storage Type unknown         5    Deleted
// Storage Type unknown         6    Deleted
//
// # of usage entries           7          5
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastEntriesAreOfflineAndUnknown_MoveOfflineEntryFailed) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1,       kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1,   kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3,   kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoStorageTypeUnknown};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 5;  // kUsageEntryInfoOfflineLicense1
  uint32_t last_valid_usage_entry_number =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[last_valid_usage_entry_number].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      last_valid_usage_entry_number));

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_valid_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(MOVE_USAGE_ENTRY_UNKNOWN_ERROR));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(last_valid_usage_entry_number + 1, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoOfflineLicense2,
              kUsageEntryInfoOfflineLicense3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last
// 2. Last few entries are of storage type unknown.
// 3. Entry that preceeds those in (2) is an offline license and calling
//    OEMCrypto_MoveUsageEntry on it will fail.
//
// Attempting to delete the entry in (1) will result in:
// a. Entries of storage type unknown at the end will be deleted.
// b. The offline usage entry that preceeds the entries in (a) will
//    not be deleted/moved if the OEMCrypto_MoveUsageEntry operation fails.
// c. The usage entry requested to be deleted will be marked as
//     storage type unknown.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2  Deleted/storage type unknown
// Secure stop 2                3          3
// Secure stop 3                4          4
// Storage Type unknown         5    Deleted
// Storage Type unknown         6    Deleted
//
// # of usage entries           7          5
TEST_F(UsageTableHeaderTest,
       DeleteEntry_LastEntriesAreSecureStopAndUnknown_MoveOfflineEntryFailed) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1,   kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1,       kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3,       kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoStorageTypeUnknown};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 5;  // kUsageEntryInfoOfflineLicense1
  uint32_t last_valid_usage_entry_number =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_valid_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(MOVE_USAGE_ENTRY_UNKNOWN_ERROR));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<2>(kProviderSessionToken),
                SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
                SetArgPointee<5>(kUsageEntry),
                SetArgPointee<6>(last_valid_usage_entry_number), Return(true)));
  EXPECT_CALL(
      *crypto_session_,
      ShrinkUsageTableHeader(last_valid_usage_entry_number + 1, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoSecureStop2,
              kUsageEntryInfoSecureStop3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last entry is an offline license.
//
// Attempting to delete the entry in (1) will result in:
// a. The usage entry requested to be deleted will be replaced with the last
//    entry.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2    Deleted
// Offline License 2            3          3
// Offline License 3            4          2
//
// # of usage entries           5          4
TEST_F(UsageTableHeaderTest, DeleteEntry_LastEntryIsOffline) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense1
  uint32_t last_usage_entry_number =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[last_usage_entry_number].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      last_usage_entry_number));

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*crypto_session_,
              ShrinkUsageTableHeader(last_usage_entry_number, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoOfflineLicense3, kUsageEntryInfoOfflineLicense2)))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoOfflineLicense3, kUsageEntryInfoOfflineLicense2,
              kUsageEntryInfoOfflineLicense3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      kUsageEntryInfoOfflineLicense3.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_to_be_deleted, usage_entry_number);
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last entry is a secure stop.
//
// Attempting to delete the entry in (1) will result in:
// a. The usage entry requested to be deleted will be replaced with the last
//    entry.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2    Deleted
// Secure stop 2                3          3
// Secure stop 3                4          2
//
// # of usage entries           5          4
TEST_F(UsageTableHeaderTest, DeleteEntry_LastEntryIsSecureStop) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1, kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoSecureStop1
  uint32_t last_usage_entry_number =
      usage_entry_info_vector.size() - 1;  // kUsageEntryInfoSecureStop3
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*crypto_session_,
              ShrinkUsageTableHeader(last_usage_entry_number, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillRepeatedly(
          DoAll(SetArgPointee<2>(kProviderSessionToken),
                SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
                SetArgPointee<5>(kUsageEntry),
                SetArgPointee<6>(last_usage_entry_number), Return(true)));

  EXPECT_CALL(*device_files_,
              DeleteUsageInfo(kUsageEntryInfoSecureStop3.usage_info_file_name,
                              kProviderSessionToken))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     kUsageEntryInfoSecureStop3.usage_info_file_name,
                     kUsageEntryInfoSecureStop3.key_set_id, kAnotherUsageEntry,
                     usage_entry_number_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoSecureStop3, kUsageEntryInfoSecureStop2)))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoSecureStop3, kUsageEntryInfoSecureStop2,
              kUsageEntryInfoSecureStop3)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last few entries are of storage type unknown.
// 3. Entry that preceeds those in (2) is an offline license.
//
// Attempting to delete the entry in (1) will result in:
// a. The entry being deleted and replaced with the offline entry in (3).
// b. The entries with unknown storage type in (2) will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Secure Stop 1                0          0
// Storage Type unknown         1          1
// Offline License 1            2    Deleted
// Offline License 2            3          3
// Offline License 3            4          2
// Storage Type unknown         5    Deleted
// Storage Type unknown         6    Deleted
//
// # of usage entries           7          4
TEST_F(UsageTableHeaderTest, DeleteEntry_LastEntriesAreOfflineAndUnknknown) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoSecureStop1,       kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoOfflineLicense1,   kUsageEntryInfoOfflineLicense2,
      kUsageEntryInfoOfflineLicense3,   kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoStorageTypeUnknown};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 5;  // kUsageEntryInfoOfflineLicense1
  uint32_t last_valid_usage_entry_number =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoOfflineLicense3
  metrics::CryptoMetrics metrics;

  EXPECT_TRUE(device_files_->StoreLicense(
      usage_entry_info_vector[last_valid_usage_entry_number].key_set_id,
      kActiveLicenseState, kPsshData, kKeyRequest, kKeyResponse,
      kKeyRenewalRequest, kKeyRenewalResponse, kReleaseServerUrl,
      kPlaybackStartTime, kPlaybackStartTime + kPlaybackDuration,
      kGracePeriodEndTime, kEmptyAppParameters, kUsageEntry,
      last_valid_usage_entry_number));

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_valid_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*crypto_session_,
              ShrinkUsageTableHeader(last_valid_usage_entry_number, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoOfflineLicense3, kUsageEntryInfoOfflineLicense2)))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoSecureStop1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoOfflineLicense3, kUsageEntryInfoOfflineLicense2,
              kUsageEntryInfoOfflineLicense3, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));

  DeviceFiles::LicenseState license_state = DeviceFiles::kLicenseStateUnknown;
  CdmInitData pssh_data;
  CdmKeyMessage key_request;
  CdmKeyResponse key_response;
  CdmKeyMessage key_renewal_request;
  CdmKeyResponse key_renewal_response;
  std::string release_server_url;
  int64_t playback_start_time;
  int64_t last_playback_time;
  int64_t grace_period_end_time;
  CdmAppParameterMap app_parameters;
  CdmUsageEntry usage_entry;
  uint32_t usage_entry_number = ~0;

  EXPECT_TRUE(device_files_->RetrieveLicense(
      kUsageEntryInfoOfflineLicense3.key_set_id, &license_state, &pssh_data,
      &key_request, &key_response, &key_renewal_request, &key_renewal_response,
      &release_server_url, &playback_start_time, &last_playback_time,
      &grace_period_end_time, &app_parameters, &usage_entry,
      &usage_entry_number));
  EXPECT_EQ(kActiveLicenseState, license_state);
  EXPECT_EQ(kPsshData, pssh_data);
  EXPECT_EQ(kKeyRequest, key_request);
  EXPECT_EQ(kKeyResponse, key_response);
  EXPECT_EQ(kKeyRenewalRequest, key_renewal_request);
  EXPECT_EQ(kKeyRenewalResponse, key_renewal_response);
  EXPECT_EQ(kReleaseServerUrl, release_server_url);
  EXPECT_EQ(kPlaybackStartTime, playback_start_time);
  EXPECT_EQ(kPlaybackStartTime + kPlaybackDuration, last_playback_time);
  EXPECT_EQ(kGracePeriodEndTime, grace_period_end_time);
  EXPECT_EQ(kEmptyAppParameters.size(), app_parameters.size());
  EXPECT_EQ(kAnotherUsageEntry, usage_entry);
  EXPECT_EQ(usage_entry_number_to_be_deleted, usage_entry_number);
}

// Initial Test state:
// 1. Usage entry to be deleted is not last.
// 2. Last few entries are of storage type unknown.
// 3. Entry that preceeds those in (2) is a secure stop.
//
// Attempting to delete the entry in (1) will result in:
// a. The entry being deleted and replaced with the secure stop entry in (3).
// b. The entries with unknown storage type in (2) will be deleted.
//
// Storage type             Usage entries
//                       at start     at end
// =============         ========     ======
// Offline License 1            0          0
// Storage Type unknown         1          1
// Secure stop 1                2    Deleted
// Secure stop 2                3          3
// Secure stop 3                4          2
// Storage Type unknown         5    Deleted
// Storage Type unknown         6    Deleted
//
// # of usage entries           7          4
TEST_F(UsageTableHeaderTest, DeleteEntry_LastEntriesAreSecureStopAndUnknknown) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1,   kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoSecureStop1,       kUsageEntryInfoSecureStop2,
      kUsageEntryInfoSecureStop3,       kUsageEntryInfoStorageTypeUnknown,
      kUsageEntryInfoStorageTypeUnknown};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  Init(kSecurityLevelL1, kUsageTableHeader, usage_entry_info_vector);
  uint32_t usage_entry_number_to_be_deleted =
      usage_entry_info_vector.size() - 5;  // kUsageEntryInfoSecureStop1
  uint32_t last_valid_usage_entry_number =
      usage_entry_info_vector.size() - 3;  // kUsageEntryInfoSecureStop3
  metrics::CryptoMetrics metrics;

  EXPECT_CALL(*crypto_session_, Open(kLevelDefault))
      .Times(2)
      .WillRepeatedly(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              LoadUsageEntry(last_valid_usage_entry_number, kUsageEntry))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_,
              MoveUsageEntry(usage_entry_number_to_be_deleted))
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, UpdateUsageEntry(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kAnotherUsageTableHeader),
                      SetArgPointee<1>(kAnotherUsageEntry), Return(NO_ERROR)));
  EXPECT_CALL(*crypto_session_,
              ShrinkUsageTableHeader(last_valid_usage_entry_number, NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<1>(kAnotherUsageTableHeader), Return(NO_ERROR)));

  EXPECT_CALL(*device_files_,
              RetrieveUsageInfoByKeySetId(
                  kUsageEntryInfoSecureStop3.usage_info_file_name,
                  kUsageEntryInfoSecureStop3.key_set_id, NotNull(), NotNull(),
                  NotNull(), NotNull(), NotNull()))
      .WillRepeatedly(
          DoAll(SetArgPointee<2>(kProviderSessionToken),
                SetArgPointee<3>(kKeyRequest), SetArgPointee<4>(kKeyResponse),
                SetArgPointee<5>(kUsageEntry),
                SetArgPointee<6>(last_valid_usage_entry_number), Return(true)));

  EXPECT_CALL(*device_files_,
              DeleteUsageInfo(kUsageEntryInfoSecureStop3.usage_info_file_name,
                              kProviderSessionToken))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageInfo(kProviderSessionToken, kKeyRequest, kKeyResponse,
                     kUsageEntryInfoSecureStop3.usage_info_file_name,
                     kUsageEntryInfoSecureStop3.key_set_id, kAnotherUsageEntry,
                     usage_entry_number_to_be_deleted))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoSecureStop3, kUsageEntryInfoSecureStop2)))
      .WillOnce(Return(true));

  EXPECT_CALL(
      *device_files_,
      StoreUsageTableInfo(
          kAnotherUsageTableHeader,
          UnorderedElementsAre(
              kUsageEntryInfoOfflineLicense1, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoSecureStop3, kUsageEntryInfoSecureStop2,
              kUsageEntryInfoSecureStop3, kUsageEntryInfoStorageTypeUnknown,
              kUsageEntryInfoStorageTypeUnknown)))
      .WillOnce(Return(true));

  EXPECT_EQ(NO_ERROR,
            usage_table_header_->DeleteEntry(usage_entry_number_to_be_deleted,
                                             device_files_, &metrics));
}

// If the crypto session says the usage table header is stale, init should fail.
TEST_F(UsageTableHeaderTest, StaleHeader) {
  std::vector<CdmUsageEntryInfo> usage_entry_info_vector;
  const CdmUsageEntryInfo usage_entry_info_array[] = {
      kUsageEntryInfoOfflineLicense1, kUsageEntryInfoSecureStop1,
      kUsageEntryInfoStorageTypeUnknown, kUsageEntryInfoOfflineLicense2};
  ToVector(usage_entry_info_vector, usage_entry_info_array,
           sizeof(usage_entry_info_array));

  EXPECT_CALL(*device_files_, RetrieveUsageTableInfo(NotNull(), NotNull()))
      .WillOnce(DoAll(SetArgPointee<0>(kUsageTableHeader),
                      SetArgPointee<1>(usage_entry_info_vector),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, LoadUsageTableHeader(kUsageTableHeader))
      .WillOnce(Return(LOAD_USAGE_HEADER_GENERATION_SKEW));
  EXPECT_CALL(*crypto_session_, CreateUsageTableHeader(NotNull()))
      .WillOnce(
          DoAll(SetArgPointee<0>(kEmptyUsageTableHeader), Return(NO_ERROR)));
  EXPECT_CALL(*device_files_, DeleteAllLicenses()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, DeleteAllUsageInfo()).WillOnce(Return(true));
  EXPECT_CALL(*device_files_, StoreUsageTableInfo(kEmptyUsageTableHeader,
                                                  kEmptyUsageEntryInfoVector))
      .WillOnce(Return(true));

  EXPECT_TRUE(usage_table_header_->Init(kSecurityLevelL1, crypto_session_));
}

}  // namespace wvcdm
