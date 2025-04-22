// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// RMT MDU encoder
///
/// \file   rmt_mdu_encoder.c
/// \author Vincent Hamp
/// \date   13/01/2023

#include "rmt_mdu_encoder.h"
#include <esp_attr.h>
#include <esp_check.h>
#include <esp_heap_caps.h>
#include <limits.h>

#if __has_include(<esp_linux_helper.h>)
#  include <esp_linux_helper.h>
#endif

#if defined(CONFIG_RMT_TX_ISR_HANDLER_IN_IRAM)
#  define RMT_IRAM_ATTR IRAM_ATTR
#else
#  define RMT_IRAM_ATTR
#endif

// https://github.com/espressif/esp-idf/issues/13032
#if !defined(RMT_MEM_ALLOC_CAPS)
#  if CONFIG_RMT_ISR_IRAM_SAFE || CONFIG_RMT_RECV_FUNC_IN_IRAM
#    define RMT_MEM_ALLOC_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#  else
#    define RMT_MEM_ALLOC_CAPS MALLOC_CAP_DEFAULT
#  endif
#endif

static char const* TAG = "rmt";

/// MDU encoder
typedef struct {
  rmt_encoder_t base;
  rmt_encoder_t* copy_encoder;
  rmt_symbol_word_t one_symbols[2u];
  rmt_symbol_word_t zero_symbols[2u];
  rmt_symbol_word_t ackreq_symbols[2u];
  size_t num_preamble_symbols;
  size_t num_ackreq_symbols;
  size_t last_bit_index;
  size_t last_byte_index;
  enum { Preamble, Start, Data, End, Ackreq } state;
  bool last_level;
} rmt_mdu_encoder_t;

/// Encode single bit
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \param  symbols       Symbols representing current bit
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR
rmt_encode_mdu_bit(rmt_mdu_encoder_t* mdu_encoder,
                   rmt_channel_handle_t channel,
                   rmt_encode_state_t* ret_state,
                   rmt_symbol_word_t const* symbols) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encoder_handle_t copy_encoder = mdu_encoder->copy_encoder;

  size_t const tmp = copy_encoder->encode(copy_encoder,
                                          channel,
                                          &symbols[mdu_encoder->last_level],
                                          sizeof(rmt_symbol_word_t),
                                          &state);
  if (tmp) mdu_encoder->last_level = !mdu_encoder->last_level;
  encoded_symbols += tmp;

  *ret_state = state;
  return encoded_symbols;
}

/// Encode preamble
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR
rmt_encode_mdu_preamble(rmt_mdu_encoder_t* mdu_encoder,
                        rmt_channel_handle_t channel,
                        size_t data_size,
                        rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encoder_handle_t copy_encoder = mdu_encoder->copy_encoder;

  while (mdu_encoder->state == Preamble) {
    size_t const tmp =
      copy_encoder->encode(copy_encoder,
                           channel,
                           &mdu_encoder->one_symbols[mdu_encoder->last_level],
                           sizeof(rmt_symbol_word_t),
                           &state);
    if (tmp) mdu_encoder->last_level = !mdu_encoder->last_level;
    encoded_symbols += tmp;
    mdu_encoder->last_byte_index += tmp;
    if (state & RMT_ENCODING_COMPLETE &&
        mdu_encoder->last_byte_index >= mdu_encoder->num_preamble_symbols) {
      mdu_encoder->last_byte_index = 0u;
      mdu_encoder->state = Start;
    }
    if (state & RMT_ENCODING_MEM_FULL) break;
  }

  *ret_state = state;
  return encoded_symbols;
}

/// Encode packet start
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR
rmt_encode_mdu_start(rmt_mdu_encoder_t* mdu_encoder,
                     rmt_channel_handle_t channel,
                     rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  encoded_symbols +=
    rmt_encode_mdu_bit(mdu_encoder, channel, &state, mdu_encoder->zero_symbols);
  if (state & RMT_ENCODING_COMPLETE) mdu_encoder->state = Data;
  *ret_state = state;
  return encoded_symbols;
}

/// Encode data
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  primary_data  App data to be encoded into RMT symbols
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR rmt_encode_mdu_data(rmt_mdu_encoder_t* mdu_encoder,
                                                rmt_channel_handle_t channel,
                                                void const* primary_data,
                                                size_t data_size,
                                                rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encoder_handle_t copy_encoder = mdu_encoder->copy_encoder;
  uint8_t const* data = (uint8_t const*)primary_data;

  while (mdu_encoder->last_bit_index < 8u) {
    bool const bit = data[mdu_encoder->last_byte_index] &
                     (1u << (7u - mdu_encoder->last_bit_index));
    rmt_symbol_word_t const* symbols =
      bit ? mdu_encoder->one_symbols : mdu_encoder->zero_symbols;
    size_t const tmp = copy_encoder->encode(copy_encoder,
                                            channel,
                                            &symbols[mdu_encoder->last_level],
                                            sizeof(rmt_symbol_word_t),
                                            &state);
    if (tmp) mdu_encoder->last_level = !mdu_encoder->last_level;
    encoded_symbols += tmp;
    mdu_encoder->last_bit_index += tmp;
    if (state & RMT_ENCODING_MEM_FULL) goto out;
  }

  mdu_encoder->last_bit_index = 0u;
  if (++mdu_encoder->last_byte_index < data_size) mdu_encoder->state = Start;
  else {
    mdu_encoder->last_byte_index = 0u;
    mdu_encoder->state = End;
  }

out:
  *ret_state = state;
  return encoded_symbols;
}

/// Encode packet end
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR rmt_encode_mdu_end(rmt_mdu_encoder_t* mdu_encoder,
                                               rmt_channel_handle_t channel,
                                               rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  encoded_symbols +=
    rmt_encode_mdu_bit(mdu_encoder, channel, &state, mdu_encoder->one_symbols);
  if (state & RMT_ENCODING_COMPLETE) mdu_encoder->state = Ackreq;
  *ret_state = state;
  return encoded_symbols;
}

/// Encode ackreq
///
/// \param  mdu_encoder   MDU encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR
rmt_encode_mdu_ackreq(rmt_mdu_encoder_t* mdu_encoder,
                      rmt_channel_handle_t channel,
                      rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encoder_handle_t copy_encoder = mdu_encoder->copy_encoder;

  // Skip
  if (mdu_encoder->num_ackreq_symbols == 0u) {
    state |= RMT_ENCODING_COMPLETE;
    mdu_encoder->last_bit_index = mdu_encoder->last_byte_index = 0u;
    mdu_encoder->state = Preamble;
    mdu_encoder->last_level = false;
  }
  // Encode ackreq symbols
  else
    while (mdu_encoder->state == Ackreq) {
      size_t const tmp = copy_encoder->encode(
        copy_encoder,
        channel,
        &mdu_encoder->ackreq_symbols[mdu_encoder->last_level],
        sizeof(rmt_symbol_word_t),
        &state);
      if (tmp) mdu_encoder->last_level = !mdu_encoder->last_level;
      encoded_symbols += tmp;
      mdu_encoder->last_byte_index += tmp;
      if (state & RMT_ENCODING_COMPLETE &&
          mdu_encoder->last_byte_index >= mdu_encoder->num_ackreq_symbols) {
        mdu_encoder->last_bit_index = mdu_encoder->last_byte_index = 0u;
        mdu_encoder->state = Preamble;
        mdu_encoder->last_level = false;
      }
      if (state & RMT_ENCODING_MEM_FULL) break;
    }

  *ret_state = state;
  return encoded_symbols;
}

/// Encode the user data into RMT symbols and write into RMT memory
///
/// \param  encoder       Encoder handle
/// \param  channel       RMT TX channel handle
/// \param  primary_data  App data to be encoded into RMT symbols
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t RMT_IRAM_ATTR rmt_encode_mdu(rmt_encoder_t* encoder,
                                           rmt_channel_handle_t channel,
                                           void const* primary_data,
                                           size_t data_size,
                                           rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  rmt_encode_state_t session_state = RMT_ENCODING_RESET;
  rmt_mdu_encoder_t* mdu_encoder =
    __containerof(encoder, rmt_mdu_encoder_t, base);

  switch (mdu_encoder->state) {
    case Preamble:
      encoded_symbols += rmt_encode_mdu_preamble(
        mdu_encoder, channel, data_size, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      // fallthrough

    case Start:
    start:
      encoded_symbols +=
        rmt_encode_mdu_start(mdu_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      // fallthrough

    case Data:
      encoded_symbols += rmt_encode_mdu_data(
        mdu_encoder, channel, primary_data, data_size, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      if (mdu_encoder->state < End) goto start;
      // fallthrough

    case End:
      encoded_symbols +=
        rmt_encode_mdu_end(mdu_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      // fallthrough

    case Ackreq:
      encoded_symbols +=
        rmt_encode_mdu_ackreq(mdu_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_COMPLETE) state |= RMT_ENCODING_COMPLETE;
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      // fallthrough
  }

  *ret_state = state;
  return encoded_symbols;
}

/// Delete RMT MDU encoder
///
/// \param  encoder             RMT encoder handle
/// \retval ESP_OK              Delete RMT MDU encoder successfully
/// \retval ESP_ERR_INVALID_ARG Delete RMT MDU encoder failed because of invalid
///                             argument
/// \retval ESP_FAIL            Delete RMT MDU encoder failed because of other
///                             error
static esp_err_t rmt_del_mdu_encoder(rmt_encoder_t* encoder) {
  rmt_mdu_encoder_t* mdu_encoder =
    __containerof(encoder, rmt_mdu_encoder_t, base);
  rmt_del_encoder(mdu_encoder->copy_encoder);
  free(mdu_encoder);
  return ESP_OK;
}

/// Reset RMT MDU encoder
///
/// \param  encoder             RMT encoder handle
/// \retval ESP_OK              Reset RMT MDU encoder successfully
/// \retval ESP_ERR_INVALID_ARG Reset RMT MDU encoder failed because of invalid
///                             argument
/// \retval ESP_FAIL            Reset RMT MDU encoder failed because of other
///                             error
static esp_err_t RMT_IRAM_ATTR rmt_mdu_encoder_reset(rmt_encoder_t* encoder) {
  rmt_mdu_encoder_t* mdu_encoder =
    __containerof(encoder, rmt_mdu_encoder_t, base);
  rmt_encoder_reset(mdu_encoder->copy_encoder);
  mdu_encoder->last_bit_index = mdu_encoder->last_byte_index = 0u;
  mdu_encoder->state = Preamble;
  mdu_encoder->last_level = false;
  return ESP_OK;
}

/// Helper function to set symbols
///
/// \param  symbols   Symbols representing current bit
/// \param  duration  Duration
static void set_symbols(rmt_symbol_word_t* symbols, uint32_t duration) {
  uint32_t const duration0 = duration / 2u;
  uint32_t const duration1 = (duration + 1u) / 2u;
  symbols[0u] = (rmt_symbol_word_t){
    .duration0 = duration0,
    .level0 = 0u,
    .duration1 = duration1,
    .level1 = 0u,
  };
  symbols[1u] = (rmt_symbol_word_t){
    .duration0 = duration0,
    .level0 = 1u,
    .duration1 = duration1,
    .level1 = 1u,
  };
}

/// Helper function to set symbols
///
/// \param  mdu_encoder     RMT MDU encoder handle
/// \param  bit1_duration   Bit1 duration
/// \param  bit0_duration   Bit0 duration
/// \param  ackreq_duration Ackreq bit duration
static void set_all_symbols(rmt_mdu_encoder_t* mdu_encoder,
                            uint32_t bit1_duration,
                            uint32_t bit0_duration,
                            uint32_t ackreq_duration) {
  set_symbols(mdu_encoder->one_symbols, bit1_duration);
  set_symbols(mdu_encoder->zero_symbols, bit0_duration);
  set_symbols(mdu_encoder->ackreq_symbols, ackreq_duration);
}

/// Create RMT MDU encoder which encodes MDU byte stream into RMT symbols
///
/// \param  config              MDU encoder configuration
/// \param  ret_encoder         Returned encoder handle
/// \retval ESP_OK              Create RMT MDU encoder successfully
/// \retval ESP_ERR_INVALID_ARG Create RMT MDU encoder failed because of
///                             invalid argument
/// \retval ESP_ERR_NO_MEM      Create RMT MDU encoder failed because out of
///                             memory
/// \retval ESP_FAIL            Create RMT MDU encoder failed because of other
///                             error
esp_err_t rmt_new_mdu_encoder(mdu_encoder_config_t const* config,
                              rmt_encoder_handle_t* ret_encoder) {
  esp_err_t ret = ESP_OK;
  rmt_mdu_encoder_t* mdu_encoder = NULL;
  ESP_GOTO_ON_FALSE(
    config && ret_encoder &&                                          //
      (config->transfer_rate >= 0u && config->transfer_rate <= 4u) && //
      (config->num_preamble >= MDU_TX_MIN_PREAMBLE_BITS &&            //
       config->num_preamble <= MDU_TX_MAX_PREAMBLE_BITS) &&           //
      (config->num_ackreq == 0u ||                                    //
       (config->num_ackreq >= MDU_TX_MIN_ACKREQ_BITS &&               //
        config->num_ackreq <= MDU_TX_MAX_ACKREQ_BITS)),               //
    ESP_ERR_INVALID_ARG,
    err,
    TAG,
    "invalid argument");
  mdu_encoder =
    heap_caps_calloc(1, sizeof(rmt_mdu_encoder_t), RMT_MEM_ALLOC_CAPS);
  ESP_GOTO_ON_FALSE(
    mdu_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for mdu encoder");

  mdu_encoder->base.encode = rmt_encode_mdu;
  mdu_encoder->base.del = rmt_del_mdu_encoder;
  mdu_encoder->base.reset = rmt_mdu_encoder_reset;

  rmt_copy_encoder_config_t copy_encoder_config = {};
  ESP_GOTO_ON_ERROR(
    rmt_new_copy_encoder(&copy_encoder_config, &mdu_encoder->copy_encoder),
    err,
    TAG,
    "create copy encoder failed");

  switch (config->transfer_rate) {
    case 0: set_all_symbols(mdu_encoder, 1200u, 2400u, 3600u); break;
    case 1: set_all_symbols(mdu_encoder, 10u, 20u, 60u); break;
    case 2: set_all_symbols(mdu_encoder, 20u, 40u, 60u); break;
    case 3: set_all_symbols(mdu_encoder, 40u, 80u, 120u); break;
    case 4: set_all_symbols(mdu_encoder, 75u, 150u, 225u); break;
  }
  mdu_encoder->num_preamble_symbols = config->num_preamble;
  mdu_encoder->num_ackreq_symbols = config->num_ackreq;

  *ret_encoder = &mdu_encoder->base;
  return ESP_OK;
err:
  if (mdu_encoder) {
    if (mdu_encoder->copy_encoder) rmt_del_encoder(mdu_encoder->copy_encoder);
    free(mdu_encoder);
  }
  return ret;
}