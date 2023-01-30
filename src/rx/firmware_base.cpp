// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive firmware base
///
/// \file   mdu/rx/firmware_base.cpp
/// \author Vincent Hamp
/// \date   12/12/2022

#include "rx/firmware_base.hpp"
#include <cstring>

namespace mdu::rx::detail {

namespace {

/// Make decoder specific Salsa20 key
///
/// \param  decoder_id  Decoder ID
/// \param  iv          Initialization vector
/// \param  master_key  Master key
/// \return Decoder specific Salsa20 key
std::array<uint8_t, 32uz> make_salsa20_key(uint32_t decoder_id,
                                           std::span<uint8_t const, 8uz> iv,
                                           char const* master_key) {
  ECRYPT_ctx ctx;
  std::array<uint8_t, 32uz> tmp;
  std::memcpy(&tmp[0uz], master_key, 32u);
  ECRYPT_keysetup(&ctx, &tmp[0uz], 256u, 64u);
  ECRYPT_ivsetup(&ctx, &iv[0uz]);
  for (auto i{0uz}; i < 32uz; i += 4uz) {
    tmp[i + 3uz] = static_cast<uint8_t>(decoder_id >> 24u);
    tmp[i + 2uz] = static_cast<uint8_t>(decoder_id >> 16u);
    tmp[i + 1uz] = static_cast<uint8_t>(decoder_id >> 8u);
    tmp[i + 0uz] = static_cast<uint8_t>(decoder_id >> 0u);
  }
  std::array<uint8_t, 32uz> crypt_key;
  ECRYPT_encrypt_bytes(&ctx, &tmp[0uz], &crypt_key[0uz], 32u);
  return crypt_key;
}

}  // namespace

/// Make decoder specific Salsa20 context
///
/// \param  decoder_id  Decoder ID
/// \param  iv          Initialization vector
/// \param  master_key  Master key
/// \return Decoder specific Salsa20 context
ECRYPT_ctx make_salsa20_context(uint32_t decoder_id,
                                std::span<uint8_t const, 8uz> iv,
                                char const* master_key) {
  auto const crypt_key{make_salsa20_key(decoder_id, iv, master_key)};
  ECRYPT_ctx ctx;
  ECRYPT_keysetup(&ctx, &crypt_key[0uz], 256u, 64u);
  ECRYPT_ivsetup(&ctx, &iv[0uz]);
  return ctx;
}

}  // namespace mdu::rx::detail