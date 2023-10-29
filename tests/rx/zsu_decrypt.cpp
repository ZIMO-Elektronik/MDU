#include <filesystem>
#include <fstream>
#include <iostream>
#include <range/v3/all.hpp>
#include <ztl/math.hpp>
#include "../utility.hpp"
#include "zsu_test.hpp"

#if defined(MDU_TESTS_MASTER_KEY)
using namespace ::testing;

namespace {

using namespace ranges;

// Read vector 64 bytes at a time
struct Reader64 {
  Reader64(std::vector<uint8_t> const& data) : data_{data} {}

  std::optional<std::vector<uint8_t>> chunk() {
    size_t const n{
      pos_ < size(data_) ? std::min<size_t>(size(data_) - pos_, 64u) : 0u};
    if (!n) return {};
    std::vector<uint8_t> retval(64u);
    std::copy_n(&data_[pos_], n, begin(retval));
    pos_ += n;
    return retval;
  }

private:
  std::vector<uint8_t> data_{};
  size_t pos_{};
};

// Read decoder ID from .zsu
uint32_t zsu2decoder_id(std::filesystem::path zsu_path) {
  assert(zsu_path.extension() == std::filesystem::path{".zsu"});
  std::ifstream fr{zsu_path};
  std::istream& is{fr};
  is >> std::noskipws;
  auto ids{ranges::istream_view<char>{is} |
           views::take_while([](auto c) { return c != ':'; }) |
           views::split(';') | views::drop_exactly(1u) |
           views::for_each([](auto&& r) {
             return r | views::split('\t') | views::take_exactly(1u);
           }) |
           views::transform([](auto&& r) {
             std::string tmp;
             ranges::for_each(r, [&tmp](auto c) { tmp += c; });
             return std::stoull(tmp);
           })};
  return *begin(ids);
}

// Read Salsa20 initialization vector from .zsu
auto zsu2initialization_vector(std::filesystem::path zsu_path) {
  std::ifstream fr{zsu_path};
  std::istream& is{fr};
  is >> std::noskipws;
  auto ivs{ranges::istream_view<char>{is} |
           views::take_while([](auto c) { return c != ':'; }) |
           views::split(';') | views::drop_exactly(1u) |
           views::for_each([](auto&& r) {
             return r | views::split('\t') | views::drop_exactly(8u);
           }) |
           views::transform([](auto&& r) {
             std::string tmp;
             ranges::for_each(r, [&tmp](auto c) { tmp += c; });
             return std::stoull(tmp);
           })};
  auto tmp{*begin(ivs)};
  std::array<uint8_t, 8uz> iv;
  for (auto i{8u}; i-- > 0u;) {
    iv[i] = tmp & 0xFFu;
    tmp >>= 8u;
  }
  return iv;
}

// Read encrypted binary from .zsu
std::vector<uint8_t> zsu2encrypted_bin(std::filesystem::path zsu_path) {
  assert(zsu_path.extension() == std::filesystem::path{".zsu"});
  std::ifstream fr{zsu_path, std::ios::binary};
  if (!fr.is_open()) return {};
  fr.seekg(0, fr.end);
  auto const length{fr.tellg()};
  std::vector<uint8_t> zsu(length);
  fr.seekg(0);
  fr.read(reinterpret_cast<char*>(&zsu[0u]), length);
  auto const colon{std::find(begin(zsu), end(zsu), ':')};
  auto const pos{ztl::align(16u, colon - begin(zsu) + 1u)};
  std::vector<uint8_t> encrypted_bin;
  for (auto i{pos}; i < size(zsu); ++i)
    encrypted_bin.push_back(zsu[i]);
  return encrypted_bin;
}

// Read binary from .zsu
std::vector<uint8_t> zsu2bin(std::filesystem::path zsu_path) {
  // Make Salsa20 context
  auto const decoder_id{zsu2decoder_id(zsu_path)};
  auto const iv{zsu2initialization_vector(zsu_path)};
  auto ctx{mdu::rx::detail::make_salsa20_context(
    decoder_id, iv, MDU_TESTS_MASTER_KEY)};

  // Run decryption
  Reader64 reader{zsu2encrypted_bin(zsu_path)};
  std::optional<std::vector<uint8_t>> chunk;
  std::vector<uint8_t> retval;
  while ((chunk = reader.chunk())) {
    std::array<uint8_t, 64uz> tmp{};
    ECRYPT_decrypt_bytes(&ctx, &(*chunk)[0u], &tmp[0u], 64u);
    for (auto v : tmp)
      retval.push_back(v);
  }

  return retval;
}

// Read binary from .bin
std::vector<uint8_t> read_bin(std::filesystem::path bin_path) {
  assert(bin_path.extension() == std::filesystem::path{".bin"});
  std::ifstream fr{bin_path, std::ios::binary};
  if (!fr.is_open()) return {};
  fr.seekg(0, fr.end);
  auto const length{fr.tellg()};
  std::vector<uint8_t> bin(length);
  fr.seekg(0);
  fr.read(reinterpret_cast<char*>(&bin[0u]), length);
  return bin;
}

// Compare bin .zsu with .bin
bool zsu_equals_bin(std::filesystem::path zsu_path,
                    std::filesystem::path bin_path) {
  auto const zsu_bin{zsu2bin(zsu_path)};
  auto const bin{read_bin(bin_path)};
  auto const smaller{std::min(size(bin), size(zsu_bin))};
  for (auto i{16u}; i < smaller; ++i) {
    if (bin[i] == 0u && zsu_bin[i] == 255u) continue;
    if (bin[i] != zsu_bin[i]) return false;
  }
  return true;
}

}  // namespace

TEST_F(ReceiveZsuTest, decrypt) {
  auto const cwd{source_location_parent_path()};
  EXPECT_TRUE(zsu_equals_bin(cwd / "./firmwares/firmware0.zsu",
                             cwd / "./firmwares/firmware0.bin"));
  EXPECT_TRUE(zsu_equals_bin(cwd / "./firmwares/firmware1.zsu",
                             cwd / "./firmwares/firmware1.bin"));
  EXPECT_TRUE(zsu_equals_bin(cwd / "./firmwares/firmware2.zsu",
                             cwd / "./firmwares/firmware2.bin"));
  EXPECT_TRUE(zsu_equals_bin(cwd / "./firmwares/firmware3.zsu",
                             cwd / "./firmwares/firmware3.bin"));
  EXPECT_TRUE(zsu_equals_bin(cwd / "./firmwares/MS440_test.zsu",
                             cwd / "./firmwares/MS440_test.bin"));
}

TEST_F(ReceiveZsuTest, crc32_of_encrypted_bin) {
  auto const cwd{source_location_parent_path()};
  auto zsu_encrypted_bin{zsu2encrypted_bin(cwd / "./firmwares/MS440_test.zsu")};
  while (size(zsu_encrypted_bin) % 64u)
    zsu_encrypted_bin.push_back(0u);
  mdu::Crc32 crc32;
  crc32.next(zsu_encrypted_bin);
  uint32_t crc32_result = crc32.value();
  EXPECT_EQ(crc32.value(), 0x3CDDEDE6u);
}
#endif
