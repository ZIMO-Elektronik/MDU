#include <driver/rmt_tx.h>
#include <esp_log.h>
#include <esp_task.h>
#include <rmt_mdu_encoder.h>
#include <array>
#include <mdu/mdu.hpp>

#define RMT_GPIO_NUM GPIO_NUM_21

extern "C" void app_main() {
  printf("DCC RMT encoder example");

  // Setup RMT on GPIO21
  rmt_tx_channel_config_t chan_config{.gpio_num = RMT_GPIO_NUM,
                                      .clk_src = RMT_CLK_SRC_DEFAULT,
                                      .resolution_hz = 1'000'000u, // 1MHz
                                      .mem_block_symbols =
                                        SOC_RMT_CHANNELS_PER_GROUP *
                                        SOC_RMT_MEM_WORDS_PER_CHANNEL,
                                      .trans_queue_depth = 2uz,
                                      .intr_priority = 0,
                                      .flags = {}};
  rmt_channel_handle_t rmt_channel{};
  ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_config, &rmt_channel));
  ESP_ERROR_CHECK(rmt_enable(rmt_channel));

  // New MDU encoder
  mdu_encoder_config_t encoder_config{
    .transfer_rate = 0u, .num_preamble = 20u, .num_ackreq = 10u};
  rmt_encoder_handle_t rmt_encoder{};
  ESP_ERROR_CHECK(rmt_new_mdu_encoder(&encoder_config, &rmt_encoder));

  auto busy_packet{mdu::make_busy_packet()};
  rmt_transmit_config_t rmt_transmit_config{};
  for (;;)
    ESP_ERROR_CHECK(rmt_transmit(rmt_channel,
                                 rmt_encoder,
                                 data(busy_packet),
                                 size(busy_packet),
                                 &rmt_transmit_config));
}
