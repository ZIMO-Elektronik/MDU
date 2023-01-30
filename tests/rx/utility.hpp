#pragma once

#include <mdu/mdu.hpp>
#include "packet_builder.hpp"

PacketBuilder make_ping_packet(uint8_t decoder_id);
PacketBuilder make_ping_packet(uint32_t serial_number, uint32_t decoder_id);
PacketBuilder make_config_transfer_rate_packet(mdu::TransferRate transfer_rate);
PacketBuilder make_busy_packet();
PacketBuilder make_firmware_update_packet(uint32_t addr,
                                          std::span<uint8_t const, 64uz> chunk);
PacketBuilder
make_zpp_lc_dc_query_packet(std::span<uint8_t const, 4uz> developer_code);
PacketBuilder make_zpp_update_packet(uint32_t addr,
                                     std::span<uint8_t const> chunk);
PacketBuilder make_zpp_update_end_packet(uint32_t begin_addr,
                                         uint32_t end_addr);
PacketBuilder make_zpp_exit_packet();
