#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string sourceFile(const char* path) {
  std::ifstream source(path);
  return {std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>()};
}

TEST(OtaStagingGuard, FlashesOnlyAValidatedOpenFile) {
  const std::string flasher = sourceFile(FIRMWARE_FLASHER_SOURCE_PATH);
  ASSERT_FALSE(flasher.empty());

  // The shared open-file path must not reopen by pathname after validation;
  // otherwise a staged file could be replaced before its first erase/write.
  const size_t flashValidated = flasher.find("Result flashValidatedFile(HalFile& file");
  const size_t pathFlasher = flasher.find("Result flashFromSdPath(");
  ASSERT_NE(flashValidated, std::string::npos);
  ASSERT_NE(pathFlasher, std::string::npos);
  EXPECT_EQ(flasher.substr(flashValidated, pathFlasher - flashValidated).find("Storage.openFileForRead"),
            std::string::npos);

  const std::string sharedPathFlasher = flasher.substr(pathFlasher);
  const size_t sharedValidation = sharedPathFlasher.find("validateOpenImageFile(file");
  const size_t sharedFlash = sharedPathFlasher.find("flashValidatedFile(file");
  ASSERT_NE(sharedValidation, std::string::npos);
  ASSERT_NE(sharedFlash, std::string::npos);
  EXPECT_LT(sharedValidation, sharedFlash);
}

}  // namespace
