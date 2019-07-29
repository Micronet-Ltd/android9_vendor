// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// File class - provides a simple android specific file implementation

#include "file_store.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file_utils.h"
#include "log.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

#include <openssl/md5.h>
#include <openssl/sha.h>

namespace wvcdm {

namespace {
const char kCertificateFileNamePrefix[] = "cert";
const char kCertificateFileNameExt[] = ".bin";
const char kCertificateFileName[] = "cert.bin";

std::string GetFileNameSafeHash(const std::string& input) {
  std::vector<uint8_t> hash(MD5_DIGEST_LENGTH);
  const unsigned char* input_ptr =
      reinterpret_cast<const unsigned char*>(input.data());
  MD5(input_ptr, input.size(), &hash[0]);
  return wvcdm::Base64SafeEncode(hash);
}

std::string GetFileNameForIdentifier(const std::string path,
                                     const std::string identifier) {
  std::string file_name = path;
  std::string dir_path;
  const size_t delimiter_pos = path.rfind(kDirectoryDelimiter);
  if (delimiter_pos != std::string::npos) {
    dir_path = file_name.substr(0, delimiter_pos);
    file_name = path.substr(delimiter_pos + 1);
  }

  if (file_name == kCertificateFileName && !identifier.empty()) {
    const std::string hash = GetFileNameSafeHash(identifier);
    file_name = kCertificateFileNamePrefix + hash + kCertificateFileNameExt;
  }

  if (dir_path.empty())
    return file_name;
  else
    return dir_path + kDirectoryDelimiter + file_name;
}
}  // namespace

class File::Impl {
 public:
  Impl(FILE* file, const std::string& file_path)
      : file_(file), file_path_(file_path) {}
  virtual ~Impl() {}

  FILE* file_;
  std::string file_path_;
};

File::File(Impl* impl) : impl_(impl) {}

File::~File() {
  Close();
  delete impl_;
}

void File::Close() {
  if (impl_ && impl_->file_) {
    fflush(impl_->file_);
    fsync(fileno(impl_->file_));
    fclose(impl_->file_);
    impl_->file_ = NULL;
  }
}

ssize_t File::Read(char* buffer, size_t bytes) {
  if (impl_ && impl_->file_) {
    size_t len = fread(buffer, sizeof(char), bytes, impl_->file_);
    if (len == 0) {
      LOGW("File::Read: fread failed: %d", errno);
    }
    return len;
  }
  LOGW("File::Read: file not open");
  return -1;
}

ssize_t File::Write(const char* buffer, size_t bytes) {
  if (impl_ && impl_->file_) {
    size_t len = fwrite(buffer, sizeof(char), bytes, impl_->file_);
    if (len == 0) {
      LOGW("File::Write: fwrite failed: %d", errno);
    }
    return len;
  }
  LOGW("File::Write: file not open");
  return -1;
}

class FileSystem::Impl {};

FileSystem::FileSystem() : FileSystem(EMPTY_ORIGIN, NULL) {}
FileSystem::FileSystem(const std::string& origin, void* /* extra_data */)
    : origin_(origin) {}

FileSystem::~FileSystem() {}

File* FileSystem::Open(const std::string& in_name, int flags) {
  std::string open_flags;

  std::string name = GetFileNameForIdentifier(in_name, identifier_);

  // create the enclosing directory if it does not exist
  size_t delimiter_pos = name.rfind(kDirectoryDelimiter);
  if (delimiter_pos != std::string::npos) {
    std::string dir_path = name.substr(0, delimiter_pos);
    if ((flags & FileSystem::kCreate) && !Exists(dir_path))
      FileUtils::CreateDirectory(dir_path);
  }

  // ensure only owners has access
  mode_t old_mask = umask(077);
  if (((flags & FileSystem::kTruncate) && Exists(name)) ||
      ((flags & FileSystem::kCreate) && !Exists(name))) {
    FILE* fp = fopen(name.c_str(), "w+");
    if (fp) {
      fclose(fp);
    }
  }

  open_flags = (flags & FileSystem::kReadOnly) ? "rb" : "rb+";

  FILE* file = fopen(name.c_str(), open_flags.c_str());
  umask(old_mask);
  if (!file) {
    LOGW("File::Open: fopen failed: %d", errno);
    return NULL;
  }

  return new File(new File::Impl(file, name));
}

bool FileSystem::Exists(const std::string& path) {
  return FileUtils::Exists(GetFileNameForIdentifier(path, identifier_));
}

bool FileSystem::Remove(const std::string& path) {
  return FileUtils::Remove(GetFileNameForIdentifier(path, identifier_));
}

ssize_t FileSystem::FileSize(const std::string& in_path) {
  std::string path = GetFileNameForIdentifier(in_path, identifier_);
  struct stat buf;
  if (stat(path.c_str(), &buf) == 0)
    return buf.st_size;
  else
    return -1;
}

bool FileSystem::List(const std::string& path,
                      std::vector<std::string>* filenames) {
  return FileUtils::List(GetFileNameForIdentifier(path, origin_), filenames);
}

void FileSystem::SetOrigin(const std::string& origin) { origin_ = origin; }

void FileSystem::SetIdentifier(const std::string& identifier) {
  identifier_ = identifier;
}

}  // namespace wvcdm
