// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "cdm_session.h"
#include "crypto_key.h"
#include "metrics.pb.h"
#include "properties.h"
#include "scoped_ptr.h"
#include "service_certificate.h"
#include "string_conversions.h"
#include "test_printers.h"
#include "usage_table_header.h"
#include "wv_cdm_constants.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::Sequence;
using ::testing::StrEq;

namespace wvcdm {

namespace {

const std::string kEmptyString;

const std::string kToken = a2bs_hex(
    "0AAE02080212107E0A892DEEB021E7AF696B938BB1D5B1188B85AD9D05228E023082010A02"
    "82010100DBEDF2BFB0EC98213766E65049B9AB176FA4B1FBFBB2A0C96C87D9F2B895E0ED77"
    "93BDA057E6BC3E0CA2348BC6831E03609445CA4D418CB98EAC98FFC87AB2364CE76BA26BEE"
    "CDB0C45BD2A6FE9FD38CC5A1C26303AEEB7E9C3CAFAB0D10E46C07E50BEDAB42BF21F40BD2"
    "E055DB0B455191D6B4CEEB11B3F1AFA42B5C0CE4C96B75A5283C0E3AE049AA7CF86D1C4EF6"
    "6A9088B53BCF320ABC9B98A22C219DC109014EFEA72DA5FF2ED5D655DE7AE06EAC6C6B4191"
    "523B2CD2DC1EBFF5F08B11CFE056F2826C1323F12704EC7EBBC1AF935129E5543804492AF9"
    "23B848F4AF47B4BFB131C39DDDC99DBAEEE0F30AD2ADBBC63E60793D0876E37391008BB4DB"
    "F7020301000128DD22128002A9E571776EA9D22A1BD14248DA88E12FD859989F613360B8D2"
    "DA40AF31CC071C7A138466F0EB745E3FD664C0E1A5E4F01098B8D56C34A0158DF9916D192F"
    "841ADCA17FD630E1C0CBE652CAC6A52B6A1581BE4029CE6FAE0E04D2D2C7861187AF8299D8"
    "3E008DB9A2789672CA1DED903773D7E82B234CE2C799EB73CF80600C08F17EEDDDF369D2B8"
    "4A08292F22D1F18FE89521905E713BA674F2217881DBD7711B8C48D5FDCE6FAB51F935E293"
    "CB29191AB012B115FD2F5F23164B063D0A929C3E254BF0F4FA60051EB6B3498ED99FF77C19"
    "68E8CD83A35CEB054D05433FD0EA6AAE43C87DDA377591D1DCC1831EE130BFFF6D139A5ADA"
    "738B0F257CCE2649E71AB4050AAE020801121017DCBC27D11341D497135442A188DAA6188F"
    "89809105228E023082010A0282010100D21ADD7549D2748B3494526A9C3FB86C79376BBE8C"
    "8856F601B8D10461F77ACC7331B10DEBF365120056CDB5662D25907B74F12382F0F4A0CA47"
    "5EEA9562815C6228F6F698ADA27879D8890F2A2D96A746DDEF5316301C003519C2A2250354"
    "674169FDDA41CE14D3C52BEA7A20384515012D5952B38AA19E15E8563CC7AAA81C2122880A"
    "A370A64FEA23C53FB83AC3DB5753214730A349E07F64BF32BE7EAD30D02612AF110BB44FB0"
    "8E1D308173B327EF64D40C41639542B2D1A73C98A6607EC6C683B513A58470514106EF87AE"
    "1E7B9C695B93A104DF7437BFC4167789748A43ED208F2C1FA710793C688885EAE732A8BFDF"
    "5B423B23D75B88FC0ADC8FBDB5020301000128DD2212800372D2FB88098BA3B85B6B4354E0"
    "3767DBE2D7724663FB0A62ABF7704EA910E01F221349EE16D0152C769384050CE78520668C"
    "06CCFD3D789AF3EB69FF163615CD609169FDBE2E15A029D34AD2605625BC81844C9D1E2CE0"
    "519039F3799ADAEF86641E20B033DC16DF2E5B9A1A2A417B8BB3B7A4D9AD1A99367448587D"
    "A13DDE05A3ED9D62FA42078973B4AA40263D7BFA23F1072E94CDF323FA45F78408823E55C4"
    "F4C5C723819CF44CE6D98E50C04EC24D93B1AAB8877B9108B9CA391308E1A3645EBB0E7CAC"
    "BB40B5451560ED799421873BFB5ABB917FA60DB9C77CB8606AF7E3142626F5EA40E5CB8AA0"
    "89D8E7D6A9361935C426A4450EA8BC2E57290D3BF0A0962991D2A91B752FC80C3E7E4E5503"
    "3D71C94B325307A68815F026448F56A2741CEBEFC18E8C142F5F62BFAA67A291517DDE982D"
    "8CD5A9DF6E3D3A99B806F6D60991358C5BE77117D4F3168F3348E9A048539F892F4D783152"
    "C7A8095224AA56B78C5CF7BD1AB1B179C0C0D11E3C3BAC84C141A00191321E3ACC17242E68"
    "3C");
const std::string kWrappedKey = a2bs_hex(
    "3B84252DD84F1A710365014A114507FFFA3DD404625D61D1EEC7C3A39D72CB8D9318ADE9DA"
    "05D69F9776DAFDA49A97BC30E84CA275925DFD98CA04F7DB23465103A224852192DE232902"
    "99FF82024F5CCA7716ACA9BE0B56348BA16B9E3136D73789C842CB2ECA4820DDAAF59CCB9B"
    "FCF2B4B0E2E5199FDCEC8DEBFFE50BB03041D8E767EA3FE6834C2E79E261ABF17B68EA66E1"
    "45AD0A6B056F39C06531A9038C996BADD524E57AE7D5339F13C574E7A398C03D65FD730BAC"
    "36F25347350DD2AD69EFA4DC040DC2D9DD4F53A729839FA3496CF580F2CBD51C3522DD67BC"
    "BA4A91E89E2BD70449F28E026638920A6DF7B9A0B2C977ACC65AE845E76EF81CADAA746DAF"
    "51D4D6FCBC083BE50DA1874D6EB1A30579B23C30881D94A8E5181FE20BF8F8C5F2522B1E7D"
    "092B1E20BDE5373F40286DE15267247F88C564BD4EBF4F69B889A03C9892584DC340D87EE1"
    "DFF2942D1B7E7EBD846349575F2DE6FDEF71BB005CFBEA845D87937BEBCAFEAC785A092C0A"
    "76CE7F7A4FE2F8E43045DED5202A2A55F547BA5DE67AF9E6B2B7DC89EFAD34AC0B40BF4B8F"
    "F82F8706B9A88FB9C7A0972E4A4B6CA970BF4F086573D595E5DB8ED0FDA4F9446ACD4B119C"
    "1E949C194B042A5CFFC13043FF79F049068A67CC1EB671A10EF7DA927753C4D149E9D0000D"
    "4307008BA0AED576ACCADF0CE6758F683087F26C2E38297B8C7D78DC3F1E8F24D7B3A0BED9"
    "C066F8348FD19CDB54A92C4E944EE11E11B3B44344E0DB0E1B4BD6CF9295AB66C05454776A"
    "8FE33AF659F67718AB43ACB52E83F8C29129DCE9654E8F1EBF9DAB9E933955E24389A37DBE"
    "17BA89AC8C750B025CB2F65D5C8BF32FED87EF368F15751AA2114159B6C9C6C814D0720DA4"
    "6E885BBB764ADC250D05F70306C3190991C31439BF273A33B6D1773E4FD089F32E753FA3C7"
    "7B5ED7DB28407D87396F1F8C83B58176EDFE1F923BDB7DA0ADE58CA2BCD6E76F9463BE7A5A"
    "909BE2731241BF1436F3E6A639FC7C717445D89AA5812E4532405B0FB368FE736E22D10FFD"
    "15FACCF69FAC468B5552C7887763B96578038CCF154F333E2095BBFF71D5C1235E032174FB"
    "44EAB4A753E7A917666A400EBE4F3D2C90100155C27F4B30C8ACFEDA6EFC763EF3556874E5"
    "8A5AB0AEBBF39990F79EF4D65EC4697D7BBEEF4F32AE8C4A8A94814A9BE532B5AC902BC0C5"
    "FB0A3E661AFF5961B6E79C82CC32FA7B7B48297347503FD58B110B93208167CC1FB96AD822"
    "42F60B9D2BF9CCEE8E778A3D3A3302303FB4E33F607D46AACE49D3546A993EDC6FBEE6E19D"
    "36831D85877013C57FE335F38D5CD9C3E09C1CEE28BC92C53A364663A7C031DF43B89BAAB4"
    "AA8176900FD483AD70E3844BD15EE4F01D8BE72186BBF9E019FCEE5961166696854D1A901F"
    "9D71B69B05F75FF233DB3C37F18DCADA640F68C4386F2E528CD77B93521A4574EF399375CD"
    "2BE7B9FDC0AE62249717B7E0022BF55C0D023669DD09355EAA90E9DF9BEA309DF7561423BF"
    "1DAD177F07A442E1591553924C0F67C2E86774009825490322A6B74319B4C77AA6195CA393"
    "03A311F762FB0FD445278D9ACF26A9049C5031BE91F2C4A6BE994CA5A3CEBC2ACCF93AB1EB"
    "993A6AA6DEB152DE8C9BB0E6B37B478393B50D1AAE99C086A0ED6D93BA7DD2DEEAB58EE34B"
    "C5EE06BE238E8DE6CB44211097C5C90D5C04857918856F86B7036986C20A43153892ED9093"
    "33EF70621A98184DDAB5E14BC971CF98CF6C91A37FFA83B00AD3BCABBAAB2DEF1C52F43003"
    "E74C92B44F9205D22262FB47948654229DE1920F8EDF96A19A88A1CA1552F8856FB4CBF83B"
    "AA3348419159D207F65FCE9C1A500C6818");

class MockDeviceFiles : public DeviceFiles {
 public:
  MockDeviceFiles() : DeviceFiles(NULL) {}

  MOCK_METHOD1(Init, bool(CdmSecurityLevel));
  MOCK_METHOD4(RetrieveCertificate, bool(std::string*, std::string*,
                                         std::string*, uint32_t*));
};

class MockUsageTableHeader : public UsageTableHeader {
 public:
  MockUsageTableHeader() : UsageTableHeader() {}

  MOCK_METHOD2(UpdateEntry, CdmResponseType(CryptoSession* crypto_session,
                                            CdmUsageEntry* usage_entry));
};

class MockCryptoSession : public CryptoSession {
 public:
  MockCryptoSession(metrics::CryptoMetrics* crypto_metrics)
      : CryptoSession(crypto_metrics) {
    // By default, call the concrete implementation of GetUsageSupportType.
    ON_CALL(*this, GetUsageSupportType(_))
        .WillByDefault(
            Invoke(this, &MockCryptoSession::BaseGetUsageSupportType));
  }
  MOCK_METHOD1(GetClientToken, bool(std::string*));
  MOCK_METHOD1(GetProvisioningToken, bool(std::string*));
  MOCK_METHOD0(GetPreProvisionTokenType, CdmClientTokenType());
  MOCK_METHOD0(GetSecurityLevel, CdmSecurityLevel());
  MOCK_METHOD0(Open, CdmResponseType());
  MOCK_METHOD1(Open, CdmResponseType(SecurityLevel));
  MOCK_METHOD1(LoadCertificatePrivateKey, bool(std::string&));
  MOCK_METHOD0(DeleteAllUsageReports, CdmResponseType());
  MOCK_METHOD1(GetUsageSupportType, CdmResponseType(CdmUsageSupportType* type));
  MOCK_METHOD0(GetUsageTableHeader, UsageTableHeader*());

  CdmResponseType BaseGetUsageSupportType(CdmUsageSupportType* type) {
    return CryptoSession::GetUsageSupportType(type);
  }
};

class MockPolicyEngine : public PolicyEngine {
 public:
  MockPolicyEngine() : PolicyEngine("mock_session_id", NULL, NULL) {}

  // Leaving a place-holder for when PolicyEngine methods need to be mocked
};

class MockCdmLicense : public CdmLicense {
 public:
  MockCdmLicense(const CdmSessionId& session_id)
      : CdmLicense(session_id) {}

  MOCK_METHOD7(Init, bool(const std::string&, CdmClientTokenType,
                          const std::string&, bool, const std::string&,
                          CryptoSession*, PolicyEngine*));
  MOCK_METHOD0(provider_session_token, std::string());
};

}  // namespace

class CdmSessionTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    cdm_session_.reset(new CdmSession(NULL, &metrics_));
    // Inject testing mocks.
    license_parser_ = new MockCdmLicense(cdm_session_->session_id());
    cdm_session_->set_license_parser(license_parser_);
    crypto_session_ = new NiceMock<MockCryptoSession>(&crypto_metrics_);
    cdm_session_->set_crypto_session(crypto_session_);
    policy_engine_ = new MockPolicyEngine();
    cdm_session_->set_policy_engine(policy_engine_);
    file_handle_ = new MockDeviceFiles();
    cdm_session_->set_file_handle(file_handle_);
  }

  virtual void TearDown() {
    // Force the cdm_session_ to be deleted. This enforces a requirement that
    // the CDM session metrics exist at least as long as the CDM session.
    cdm_session_.reset();
  }

  metrics::SessionMetrics metrics_;
  scoped_ptr<CdmSession> cdm_session_;
  MockCdmLicense* license_parser_;
  metrics::CryptoMetrics crypto_metrics_;
  NiceMock<MockCryptoSession>* crypto_session_;
  MockPolicyEngine* policy_engine_;
  MockDeviceFiles* file_handle_;
  MockUsageTableHeader usage_table_header_;
};

TEST_F(CdmSessionTest, InitWithBuiltInCertificate) {
  Sequence crypto_session_seq;
  CdmSecurityLevel level = kSecurityLevelL1;
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, GetSecurityLevel())
      .InSequence(crypto_session_seq)
      .WillOnce(Return(level));
  EXPECT_CALL(*crypto_session_, GetPreProvisionTokenType())
      .WillOnce(Return(kClientTokenDrmCert));
  EXPECT_CALL(*file_handle_, RetrieveCertificate(NotNull(), NotNull(),
                                                 NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<0>(kToken), SetArgPointee<1>(kWrappedKey),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, LoadCertificatePrivateKey(StrEq(kWrappedKey)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(true));
  EXPECT_CALL(*file_handle_, Init(Eq(level))).WillOnce(Return(true));
  EXPECT_CALL(*license_parser_,
              Init(Eq(kToken), Eq(kClientTokenDrmCert), Eq(kEmptyString),
                   false, Eq(kEmptyString), Eq(crypto_session_),
                   Eq(policy_engine_)))
      .WillOnce(Return(true));

  ASSERT_EQ(NO_ERROR, cdm_session_->Init(NULL));
}

TEST_F(CdmSessionTest, InitWithCertificate) {
  Sequence crypto_session_seq;
  CdmSecurityLevel level = kSecurityLevelL1;
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, GetSecurityLevel())
      .InSequence(crypto_session_seq)
      .WillOnce(Return(level));
  EXPECT_CALL(*crypto_session_, GetPreProvisionTokenType())
      .WillOnce(Return(kClientTokenKeybox));
  EXPECT_CALL(*file_handle_, Init(Eq(level))).WillOnce(Return(true));
  EXPECT_CALL(*file_handle_, RetrieveCertificate(NotNull(), NotNull(),
                                                 NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<0>(kToken), SetArgPointee<1>(kWrappedKey),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, LoadCertificatePrivateKey(StrEq(kWrappedKey)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(true));
  EXPECT_CALL(*license_parser_,
              Init(Eq(kToken), Eq(kClientTokenDrmCert), Eq(kEmptyString),
                   false, Eq(kEmptyString), Eq(crypto_session_),
                   Eq(policy_engine_)))
      .WillOnce(Return(true));

  ASSERT_EQ(NO_ERROR, cdm_session_->Init(NULL));
}

TEST_F(CdmSessionTest, ReInitFail) {
  Sequence crypto_session_seq;
  CdmSecurityLevel level = kSecurityLevelL1;
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, GetSecurityLevel())
      .InSequence(crypto_session_seq)
      .WillOnce(Return(level));
  EXPECT_CALL(*crypto_session_, GetPreProvisionTokenType())
      .WillOnce(Return(kClientTokenKeybox));
  EXPECT_CALL(*file_handle_, Init(Eq(level))).WillOnce(Return(true));
  EXPECT_CALL(*file_handle_, RetrieveCertificate(NotNull(), NotNull(),
                                                 NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<0>(kToken), SetArgPointee<1>(kWrappedKey),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, LoadCertificatePrivateKey(StrEq(kWrappedKey)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(true));
  EXPECT_CALL(*license_parser_,
              Init(Eq(kToken), Eq(kClientTokenDrmCert), Eq(kEmptyString), false,
                   Eq(kEmptyString), Eq(crypto_session_), Eq(policy_engine_)))
      .WillOnce(Return(true));

  ASSERT_EQ(NO_ERROR, cdm_session_->Init(NULL));
  ASSERT_NE(NO_ERROR, cdm_session_->Init(NULL));
}

TEST_F(CdmSessionTest, InitFailCryptoError) {
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .WillOnce(Return(UNKNOWN_ERROR));

  ASSERT_EQ(UNKNOWN_ERROR, cdm_session_->Init(NULL));
}

TEST_F(CdmSessionTest, InitNeedsProvisioning) {
  Sequence crypto_session_seq;
  CdmSecurityLevel level = kSecurityLevelL1;
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, GetSecurityLevel())
      .InSequence(crypto_session_seq)
      .WillOnce(Return(level));
  EXPECT_CALL(*crypto_session_, GetPreProvisionTokenType())
      .WillOnce(Return(kClientTokenKeybox));
  EXPECT_CALL(*file_handle_, Init(Eq(level))).WillOnce(Return(true));
  EXPECT_CALL(*file_handle_, RetrieveCertificate(NotNull(), NotNull(),
                                                 NotNull(), _))
      .WillOnce(Return(false));

  ASSERT_EQ(NEED_PROVISIONING, cdm_session_->Init(NULL));
}

TEST_F(CdmSessionTest, UpdateUsageEntry) {
  // Setup common expectations for initializing the CdmSession object.
  Sequence crypto_session_seq;
  CdmSecurityLevel level = kSecurityLevelL1;
  EXPECT_CALL(*crypto_session_, Open(Eq(kLevelDefault)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(NO_ERROR));
  EXPECT_CALL(*crypto_session_, GetSecurityLevel())
      .InSequence(crypto_session_seq)
      .WillOnce(Return(level));
  EXPECT_CALL(*crypto_session_, GetPreProvisionTokenType())
      .WillOnce(Return(kClientTokenKeybox));
  EXPECT_CALL(*file_handle_, Init(Eq(level))).WillOnce(Return(true));
  EXPECT_CALL(*file_handle_, RetrieveCertificate(NotNull(), NotNull(),
                                                 NotNull(), _))
      .WillOnce(DoAll(SetArgPointee<0>(kToken), SetArgPointee<1>(kWrappedKey),
                      Return(true)));
  EXPECT_CALL(*crypto_session_, LoadCertificatePrivateKey(StrEq(kWrappedKey)))
      .InSequence(crypto_session_seq)
      .WillOnce(Return(true));
  EXPECT_CALL(*crypto_session_, GetUsageTableHeader())
      .WillOnce(Return(&usage_table_header_));
  EXPECT_CALL(*license_parser_,
              Init(Eq(kToken), Eq(kClientTokenDrmCert), Eq(kEmptyString),
                   false, Eq(kEmptyString), Eq(crypto_session_),
                   Eq(policy_engine_)))
      .WillOnce(Return(true));

  // Set up mocks and expectations for the UpdateUsageEntryInformation call.
  EXPECT_CALL(*crypto_session_, GetUsageSupportType(_))
      .WillRepeatedly(DoAll(
          SetArgPointee<0>(kUsageEntrySupport), Return(NO_ERROR)));
  EXPECT_CALL(*license_parser_, provider_session_token())
      .WillRepeatedly(Return("Mock provider session token"));
  EXPECT_CALL(usage_table_header_, UpdateEntry(NotNull(), NotNull()))
      .WillRepeatedly(Return(NO_ERROR));

  EXPECT_EQ(NO_ERROR, cdm_session_->Init(NULL));
  EXPECT_EQ(kUsageEntrySupport, cdm_session_->get_usage_support_type())
      << "Usage support type: " << cdm_session_->get_usage_support_type();
  EXPECT_EQ(NO_ERROR, cdm_session_->UpdateUsageEntryInformation());

  // Verify the UsageEntry metric is set.
  drm_metrics::WvCdmMetrics::SessionMetrics metrics;
  cdm_session_->GetMetrics()->Serialize(&metrics);
  std::string serialized_metrics;
  ASSERT_TRUE(metrics.SerializeToString(&serialized_metrics));
  EXPECT_GT(metrics.crypto_metrics()
            .crypto_session_update_usage_entry_time_us().size(), 0)
      << "Missing update usage entry metric. Metrics: "
      << wvcdm::b2a_hex(serialized_metrics);
}

}  // namespace wvcdm
