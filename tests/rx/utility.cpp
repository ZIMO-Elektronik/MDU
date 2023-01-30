#include "utility.hpp"

PacketBuilder make_ping_packet(uint8_t decoder_id) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::Ping)
    .data(decoder_id)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder make_ping_packet(uint32_t serial_number, uint32_t decoder_id) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::Ping)
    .data(serial_number, decoder_id)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder
make_config_transfer_rate_packet(mdu::TransferRate transfer_rate) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ConfigTransferRate)
    .data(std::to_underlying(transfer_rate))
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder make_busy_packet() {
  PacketBuilder packet;
  packet.preamble().command(mdu::Command::Busy).crc8().ackreq();
  return packet;
}

PacketBuilder
make_firmware_update_packet(uint32_t addr,
                            std::span<uint8_t const, 64uz> chunk) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::FirmwareUpdate)
    .data(addr)
    .data(chunk)
    .crc32()
    .ackreq();
  return packet;
}

PacketBuilder
make_zpp_lc_dc_query_packet(std::span<uint8_t const, 4uz> developer_code) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppLcDcQuery)
    .data(developer_code)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder make_zpp_update_packet(uint32_t addr,
                                     std::span<uint8_t const> chunk) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppUpdate)
    .data(addr)
    .data(chunk)
    .crc32()
    .ackreq();
  return packet;
}

PacketBuilder make_zpp_update_end_packet(uint32_t begin_addr,
                                         uint32_t end_addr) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppUpdateEnd)
    .data(begin_addr, end_addr)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder make_zpp_exit_packet() {
  PacketBuilder packet;
  packet.preamble().command(mdu::Command::ZppExit).crc8().ackreq();
  return packet;
}
