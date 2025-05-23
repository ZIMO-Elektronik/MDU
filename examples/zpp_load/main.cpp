#include <mdu/mdu.hpp>

#define SN 0x12345678u
#define ID 0x87654321u
#define TIMER_VALUE 42u

class ZppLoad : public mdu::rx::ZppBase {
public:
  // Ctor takes configuration struct containing serial number, decoder ID and
  // supported transfer rate
  ZppLoad()
    : mdu::rx::ZppBase{{.serial_number = SN,
                        .decoder_id = ID,
                        .transfer_rate = mdu::TransferRate::Fast}} {}

private:
  // Generate current pulse of length "us" in µs
  void ackbit(uint32_t us) const final {}

  // Read CV bit
  bool readCv(uint32_t cv_addr, uint32_t pos) const final { return {}; }

  // Write CV
  bool writeCv(uint32_t cv_addr, uint8_t byte) final { return {}; }

  // Check if ZPP is valid
  bool zppValid(std::string_view zpp_id, size_t zpp_flash_size) const final {
    return {};
  }

  // Check if load code is valid
  bool loadCodeValid(std::span<uint8_t const, 4uz> developer_code) const final {
    return {};
  }

  // Erase ZPP in the closed-interval [begin_addr, end_addr[
  bool eraseZpp(uint32_t begin_addr, uint32_t end_addr) final { return {}; }

  // Write ZPP
  bool writeZpp(uint32_t cv_addr, std::span<uint8_t const> bytes) final {
    return {};
  }

  // Update done
  bool endZpp() final { return {}; }

  // Exit (eventually perform CV reset)
  [[noreturn]] void exitZpp(bool reset_cvs) final {}

  // Timer interrupt calls receive with captured value
  void interrupt() { receive(TIMER_VALUE); }
};

void enter_zpp_update() {}

// Ctor takes SN, ID und function object hooks with void() signature
mdu::rx::entry::Point entry_point{{.serial_number = SN,
                                   .decoder_id = ID,
                                   .zpp_entry = enter_zpp_update,
                                   .zsu_entry = nullptr}};

int main() {
  ZppLoad zpp_load{};
  entry_point.verify(0uz, 0u);
}
