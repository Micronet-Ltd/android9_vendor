// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include "initialization_data.h"

#include <arpa/inet.h>
#include <string.h>

#include "buffer_reader.h"
#include "jsmn.h"
#include "log.h"
#include "properties.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

namespace {
const char kKeyFormatVersionsSeparator = '/';
const std::string kBase64String = "base64,";
const uint32_t kFourCcCbc1 = 0x63626331;
const uint32_t kFourCcCbcs = 0x63626373;

// json init data key values
const std::string kProvider = "provider";
const std::string kContentId = "content_id";
const std::string kKeyIds = "key_ids";

// Being conservative, usually we expect 6 + number of Key Ids
const int kDefaultNumJsonTokens = 128;
}  // namespace

namespace wvcdm {

// Protobuf generated classes.
using video_widevine::WidevinePsshData;
using video_widevine::WidevinePsshData_Algorithm;
using video_widevine::WidevinePsshData_Algorithm_AESCTR;

InitializationData::InitializationData(const std::string& type,
                                       const CdmInitData& data)
    : type_(type),
      is_cenc_(false),
      is_hls_(false),
      is_webm_(false),
      hls_method_(kHlsMethodNone) {
  if (type == ISO_BMFF_VIDEO_MIME_TYPE || type == ISO_BMFF_AUDIO_MIME_TYPE ||
      type == CENC_INIT_DATA_FORMAT) {
    is_cenc_ = true;
  } else if (type == WEBM_VIDEO_MIME_TYPE || type == WEBM_AUDIO_MIME_TYPE ||
             type == WEBM_INIT_DATA_FORMAT) {
    is_webm_ = true;
  } else if (type == HLS_INIT_DATA_FORMAT) {
    is_hls_ = true;
  }

  if (is_supported()) {
    if (is_cenc()) {
      ExtractWidevinePssh(data, &data_);
    } else if (is_webm()) {
      data_ = data;
    } else if (is_hls()) {
      std::string uri;
      if (ExtractHlsAttributes(data, &hls_method_, &hls_iv_, &uri)) {
        ConstructWidevineInitData(hls_method_, uri, &data_);
      }
    }
  }
}

// Parse the pssh data and return the embedded key data if it exists.
std::vector<video_widevine::SubLicense>
InitializationData::ExtractSublicenseKeys() const {
  std::vector<video_widevine::SubLicense> keys;
  WidevinePsshData cenc_header;
  if (!is_cenc_ || !cenc_header.ParseFromString(data_) ||
      cenc_header.sub_licenses().size() == 0)
    return keys;

  keys.reserve(cenc_header.sub_licenses().size());
  for (int i = 0; i < cenc_header.sub_licenses().size(); ++i) {
    keys.push_back(cenc_header.sub_licenses(i));
  }
  return keys;
}

std::vector<video_widevine::WrappedKey> InitializationData::ExtractWrappedKeys()
    const {
  std::vector<video_widevine::WrappedKey> keys;
  WidevinePsshData cenc_header;
  if (!is_cenc_ || !cenc_header.ParseFromString(data_) ||
      cenc_header.entitled_keys().size() == 0)
    return keys;
  keys.reserve(cenc_header.entitled_keys().size());
  for (int i = 0; i < cenc_header.entitled_keys().size(); ++i) {
    keys.push_back(cenc_header.entitled_keys(i));
  }
  return keys;
}

// Parse a blob of multiple concatenated PSSH atoms to extract the first
// Widevine PSSH.
bool InitializationData::ExtractWidevinePssh(const CdmInitData& init_data,
                                             CdmInitData* output) {
  BufferReader reader(reinterpret_cast<const uint8_t*>(init_data.data()),
                      init_data.length());

  // Widevine's registered system ID.
  static const uint8_t kWidevineSystemId[] = {
      0xED, 0xEF, 0x8B, 0xA9, 0x79, 0xD6, 0x4A, 0xCE,
      0xA3, 0xC8, 0x27, 0xDC, 0xD5, 0x1D, 0x21, 0xED,
  };

  // one PSSH box consists of:
  // 4 byte size of the atom, inclusive.  (0 means the rest of the buffer.)
  // 4 byte atom type, "pssh".
  // (optional, if size == 1) 8 byte size of the atom, inclusive.
  // 1 byte version, value 0 or 1.  (skip if larger.)
  // 3 byte flags, value 0.  (ignored.)
  // 16 byte system id.
  // (optional, if version == 1) 4 byte key ID count. (K)
  // (optional, if version == 1) K * 16 byte key ID.
  // 4 byte size of PSSH data, exclusive. (N)
  // N byte PSSH data.
  while (!reader.IsEOF()) {
    size_t start_pos = reader.pos();

    // atom size, used for skipping.
    uint64_t size;
    if (!reader.Read4Into8(&size)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read atom size.");
      return false;
    }
    std::vector<uint8_t> atom_type;
    if (!reader.ReadVec(&atom_type, 4)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read atom type.");
      return false;
    }

    if (size == 1) {
      if (!reader.Read8(&size)) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to read 64-bit "
            "atom size.");
        return false;
      }
    } else if (size == 0) {
      size = reader.size() - start_pos;
    }

    // "pssh"
    if (memcmp(&atom_type[0], "pssh", 4)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: PSSH literal not present.");
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to skip the rest "
            "of the atom.");
        return false;
      }
      continue;
    }

    // version
    uint8_t version;
    if (!reader.Read1(&version)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read PSSH "
          "version.");
      return false;
    }

    if (version > 1) {
      // unrecognized version - skip.
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to skip the rest "
            "of the atom.");
        return false;
      }
      continue;
    }

    // flags
    if (!reader.SkipBytes(3)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to skip the PSSH "
          "flags.");
      return false;
    }

    // system id
    std::vector<uint8_t> system_id;
    if (!reader.ReadVec(&system_id, sizeof(kWidevineSystemId))) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read system ID.");
      return false;
    }

    if (memcmp(&system_id[0], kWidevineSystemId, sizeof(kWidevineSystemId))) {
      // skip non-Widevine PSSH boxes.
      if (!reader.SkipBytes(size - (reader.pos() - start_pos))) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to skip the rest "
            "of the atom.");
        return false;
      }
      LOGV(
          "InitializationData::ExtractWidevinePssh: Skipping non-Widevine "
          "PSSH.");
      continue;
    }

    if (version == 1) {
      // v1 has additional fields for key IDs.  We can skip them.
      uint32_t num_key_ids;
      if (!reader.Read4(&num_key_ids)) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to read num key "
            "IDs.");
        return false;
      }
      if (!reader.SkipBytes(num_key_ids * 16)) {
        LOGV(
            "InitializationData::ExtractWidevinePssh: Unable to skip key IDs.");
        return false;
      }
    }

    // size of PSSH data
    uint32_t data_length;
    if (!reader.Read4(&data_length)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read PSSH data "
          "size.");
      return false;
    }

    output->clear();
    if (!reader.ReadString(output, data_length)) {
      LOGV(
          "InitializationData::ExtractWidevinePssh: Unable to read PSSH data.");
      return false;
    }

    return true;
  }

  // we did not find a matching record
  return false;
}

// Parse an EXT-X-KEY tag attribute list. Verify that Widevine supports it
// by validating KEYFORMAT and KEYFORMATVERSION attributes. Extract out
// method, IV, URI and WV init data.
//
// An example of a widevine supported attribute list from an HLS media playlist
// is,
//    "EXT-X-KEY: METHOD=SAMPLE-AES, \"
//    "URI=”data:text/plain;base64,eyANCiAgICJwcm92aWRlciI6Im1sYmFtaGJvIiwNCiAg"
//         "ICJjb250ZW50X2lkIjoiMjAxNV9UZWFycyIsDQogICAia2V5X2lkcyI6DQogICBbDQo"
//         "gICAgICAiMzcxZTEzNWUxYTk4NWQ3NWQxOThhN2Y0MTAyMGRjMjMiDQogICBdDQp9DQ"
//         "o=, \"
//    "IV=0x6df49213a781e338628d0e9c812d328e, \"
//    "KEYFORMAT=”com.widevine”, \"
//    "KEYFORMATVERSIONS=”1”"
bool InitializationData::ExtractHlsAttributes(const std::string& attribute_list,
                                              CdmHlsMethod* method,
                                              std::vector<uint8_t>* iv,
                                              std::string* uri) {
  std::string value;
  if (!ExtractQuotedAttribute(attribute_list, HLS_KEYFORMAT_ATTRIBUTE,
                              &value)) {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: Unable to read HLS "
        "keyformat value");
    return false;
  }

  if (value.compare(0, sizeof(KEY_SYSTEM) - 1, KEY_SYSTEM) != 0) {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: unrecognized HLS "
        "keyformat value: %s",
        value.c_str());
    return false;
  }

  // KEYFORMATVERSIONS is an optional parameter. If absent its
  // value defaults to "1"
  if (ExtractQuotedAttribute(attribute_list, HLS_KEYFORMAT_VERSIONS_ATTRIBUTE,
                             &value)) {
    std::vector<std::string> versions = ExtractKeyFormatVersions(value);
    bool supported = false;
    for (size_t i = 0; i < versions.size(); ++i) {
      if (versions[i].compare(HLS_KEYFORMAT_VERSION_VALUE_1) == 0) {
        supported = true;
        break;
      }
    }
    if (!supported) {
      LOGV(
          "InitializationData::ExtractHlsInitDataAtttribute: HLS keyformat "
          "version is not supported: %s",
          value.c_str());
      return false;
    }
  }

  if (!ExtractAttribute(attribute_list, HLS_METHOD_ATTRIBUTE, &value)) {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: Unable to read HLS "
        "method");
    return false;
  }

  if (value.compare(HLS_METHOD_AES_128) == 0) {
    *method = kHlsMethodAes128;
  } else if (value.compare(HLS_METHOD_SAMPLE_AES) == 0) {
    *method = kHlsMethodSampleAes;
  } else if (value.compare(HLS_METHOD_NONE) == 0) {
    *method = kHlsMethodNone;
  } else {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: HLS method "
        "unrecognized: %s",
        value.c_str());
    return false;
  }

  if (!ExtractHexAttribute(attribute_list, HLS_IV_ATTRIBUTE, iv)) {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: HLS IV attribute "
        "not present");
    return false;
  }

  if (!ExtractQuotedAttribute(attribute_list, HLS_URI_ATTRIBUTE, uri)) {
    LOGV(
        "InitializationData::ExtractHlsInitDataAtttribute: HLS URI attribute "
        "not present");
    return false;
  }

  return true;
}

// Extracts a base64 encoded string from URI data. This is then base64 decoded
// and the Json formatted init data is then parsed. The information is used
// to generate a Widevine init data protobuf (WidevineCencHeader).
//
// An example of a widevine supported json formatted init data string is,
//
// {
//   "provider":"mlbamhbo",
//   "content_id":"MjAxNV9UZWFycw==",
//   "key_ids":
//   [
//     "371e135e1a985d75d198a7f41020dc23"
//   ]
// }
bool InitializationData::ConstructWidevineInitData(
    CdmHlsMethod method, const std::string& uri, CdmInitData* init_data_proto) {
  if (!init_data_proto) {
    LOGV("InitializationData::ConstructWidevineInitData: Invalid parameter");
    return false;
  }
  if (method != kHlsMethodAes128 && method != kHlsMethodSampleAes) {
    LOGV(
        "InitializationData::ConstructWidevineInitData: Invalid method"
        " parameter");
    return false;
  }

  size_t pos = uri.find(kBase64String);
  if (pos == std::string::npos) {
    LOGV(
        "InitializationData::ConstructWidevineInitData: URI attribute "
        "unexpected format: %s",
        uri.c_str());
    return false;
  }

  std::vector<uint8_t> json_init_data =
      Base64Decode(uri.substr(pos + kBase64String.size()));
  if (json_init_data.size() == 0) {
    LOGV(
        "InitializationData::ConstructWidevineInitData: Base64 decode of json "
        "data failed");
    return false;
  }
  std::string json_string((const char*)(&json_init_data[0]),
                          json_init_data.size());

  // Parse the Json string using jsmn
  jsmn_parser parser;
  jsmntok_t tokens[kDefaultNumJsonTokens];
  jsmn_init(&parser);
  int num_of_tokens =
      jsmn_parse(&parser, json_string.c_str(), json_string.size(), tokens,
                 kDefaultNumJsonTokens);

  if (num_of_tokens <= 0) {
    LOGV(
        "InitializationData::ConstructWidevineInitData: Json parsing failed: "
        "%d",
        num_of_tokens);
    return false;
  }

  std::string provider;
  std::string content_id;
  std::vector<std::string> key_ids;

  enum JsmnParserState {
    kParseState,
    kProviderState,
    kContentIdState,
    kKeyIdsState,
  } state = kParseState;

  int number_of_key_ids = 0;

  // Extract the provider, content_id and key_ids
  for (int i = 0; i < num_of_tokens; ++i) {
    if (tokens[i].start < 0 || tokens[i].end < 0) {
      LOGV(
          "InitializationData::ConstructWidevineInitData: Invalid start or end "
          "of token");
      return false;
    }

    switch (state) {
      case kParseState:
        if (tokens[i].type == JSMN_STRING) {
          std::string token(json_string, tokens[i].start,
                            tokens[i].end - tokens[i].start);
          if (token == kProvider) {
            state = kProviderState;
          } else if (token == kContentId) {
            state = kContentIdState;
          } else if (token == kKeyIds) {
            state = kKeyIdsState;
          }
        }
        break;
      case kProviderState:
        if (tokens[i].type == JSMN_STRING) {
          provider.assign(json_string, tokens[i].start,
                          tokens[i].end - tokens[i].start);
        }
        state = kParseState;
        break;
      case kContentIdState:
        if (tokens[i].type == JSMN_STRING) {
          std::string base64_content_id(json_string, tokens[i].start,
                                        tokens[i].end - tokens[i].start);
          std::vector<uint8_t> content_id_data =
              Base64Decode(base64_content_id);
          content_id.assign(reinterpret_cast<const char*>(&content_id_data[0]),
                            content_id_data.size());
        }
        state = kParseState;
        break;
      case kKeyIdsState:
        if (tokens[i].type == JSMN_ARRAY) {
          number_of_key_ids = tokens[i].size;
        } else if (tokens[i].type == JSMN_STRING) {
          std::string key_id(a2bs_hex(json_string.substr(
              tokens[i].start, tokens[i].end - tokens[i].start)));
          if (key_id.size() == 16) key_ids.push_back(key_id);
          --number_of_key_ids;
        } else {
          state = kParseState;
        }
        if (number_of_key_ids <= 0) state = kParseState;
        break;
    }
  }

  if (provider.size() == 0) {
    LOGV("InitializationData::ConstructWidevineInitData: Invalid provider");
    return false;
  }

  if (content_id.size() == 0) {
    LOGV("InitializationData::ConstructWidevineInitData: Invalid content_id");
    return false;
  }

  if (key_ids.size() == 0) {
    LOGV("InitializationData::ConstructWidevineInitData: No key_ids present");
    return false;
  }

  // Now format as Widevine init data protobuf
  WidevinePsshData cenc_header;
  // TODO(rfrias): The algorithm is a deprecated field, but proto changes
  // have not yet been pushed to production. Set until then.
  cenc_header.set_algorithm(WidevinePsshData_Algorithm_AESCTR);
  for (size_t i = 0; i < key_ids.size(); ++i) {
    cenc_header.add_key_ids(key_ids[i]);
  }
  cenc_header.set_provider(provider);
  cenc_header.set_content_id(content_id);
  if (method == kHlsMethodAes128)
    cenc_header.set_protection_scheme(kFourCcCbc1);
  else
    cenc_header.set_protection_scheme(kFourCcCbcs);
  cenc_header.SerializeToString(init_data_proto);
  return true;
}

bool InitializationData::ExtractQuotedAttribute(
    const std::string& attribute_list, const std::string& key,
    std::string* value) {
  bool result = ExtractAttribute(attribute_list, key, value);
  if (value->size() < 2 || value->at(0) != '\"' ||
      value->at(value->size() - 1) != '\"')
    return false;
  *value = value->substr(1, value->size() - 2);
  if (value->find('\"') != std::string::npos) return false;
  return result;
}

bool InitializationData::ExtractHexAttribute(const std::string& attribute_list,
                                             const std::string& key,
                                             std::vector<uint8_t>* value) {
  std::string val;
  bool result = ExtractAttribute(attribute_list, key, &val);
  if (!result || val.size() <= 2 || val.size() % 2 != 0 || val[0] != '0' ||
      ((val[1] != 'x') && (val[1] != 'X')))
    return false;
  for (size_t i = 2; i < val.size(); ++i) {
    if (!isxdigit(val[i])) return false;
  }
  *value = a2b_hex(val.substr(2, val.size() - 2));
  return result;
}

bool InitializationData::ExtractAttribute(const std::string& attribute_list,
                                          const std::string& key,
                                          std::string* value) {
  // validate the key
  for (size_t i = 0; i < key.size(); ++i)
    if (!isupper(key[i]) && !isdigit(key[i]) && key[i] != '-') return false;

  bool found = false;
  size_t pos = 0;
  // Find the key followed by '='
  while (!found) {
    pos = attribute_list.find(key, pos);
    if (pos == std::string::npos) return false;
    pos += key.size();
    if (attribute_list[pos] != '=') continue;
    found = true;
  }

  if (attribute_list.size() <= ++pos) return false;

  size_t end_pos = pos;
  found = false;
  // Find the next comma outside the quote or end of string
  while (!found) {
    end_pos = attribute_list.find(',', end_pos);
    if (end_pos != std::string::npos && attribute_list[pos] == '\"' &&
        attribute_list[end_pos - 1] != '\"') {
      ++end_pos;
      continue;
    }

    if (end_pos == std::string::npos)
      end_pos = attribute_list.size() - 1;
    else
      --end_pos;
    found = true;
  }

  *value = attribute_list.substr(pos, end_pos - pos + 1);

  // validate the value
  for (size_t i = 0; i < value->size(); ++i)
    if (!isgraph(value->at(i))) return false;

  return true;
}

// Key format versions are individual values or multiple versions
// separated by '/'. "1" or "1/2/5"
std::vector<std::string> InitializationData::ExtractKeyFormatVersions(
    const std::string& key_format_versions) {
  std::vector<std::string> versions;
  size_t pos = 0;
  while (pos < key_format_versions.size()) {
    size_t next_pos =
        key_format_versions.find(kKeyFormatVersionsSeparator, pos);
    if (next_pos == std::string::npos) {
      versions.push_back(key_format_versions.substr(pos));
      break;
    } else {
      versions.push_back(key_format_versions.substr(pos, next_pos - pos));
      pos = next_pos + 1;
    }
  }
  return versions;
}

}  // namespace wvcdm
