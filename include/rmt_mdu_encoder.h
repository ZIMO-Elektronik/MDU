// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// RMT MDU encoder
///
/// \file   rmt_mdu_encoder.h
/// \author Vincent Hamp
/// \date   13/01/2023

#pragma once

#include <driver/rmt_encoder.h>

#ifdef __cplusplus
extern "C" {
#endif

/// MDU encoder configuration
typedef struct {
  /// Transfer rate [0, 4]
  uint8_t transfer_rate;

  /// Number of preamble bits [14, 255]
  uint8_t num_preamble;

  /// Number of ackreq bits 0 || [10, 255]
  uint8_t num_ackreq;
} mdu_encoder_config_t;

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
                              rmt_encoder_handle_t* ret_encoder);

#ifdef __cplusplus
}
#endif