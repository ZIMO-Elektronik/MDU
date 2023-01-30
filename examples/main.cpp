#include <mdu/mdu.hpp>

#define SERIAL_NUMBER 0x12345678u
#define ID 0x87654321u
#define TIMER_VALUE 42u

class ZppLoad : public mdu::rx::ZppBase {
public:
  // Ctor takes configuration struct containing serial number, decoder ID and
  // supported tranfer rate
  ZppLoad()
    : mdu::rx::ZppBase{{.serial_number = SERIAL_NUMBER,
                        .decoder_id = ID,
                        .transfer_rate = mdu::TransferRate::Fast}} {}

private:
  // Generate current pulse of length "us" in µs
  void ackbit(uint32_t us) const final {}

  // Read CV bit
  bool readCv(uint32_t addr, uint32_t position) const final {}

  // Write CV
  bool writeCv(uint32_t addr, uint8_t value) const final {}

  // Check if ZPP is valid
  bool zppValid(std::string_view zpp_id, size_t zpp_size) const final {}

  // Check if loadcode is valid
  bool loadcodeValid(std::span<uint8_t const, 4uz> developer_code) const final {
  }

  // Erase ZPP in the closed-interval [begin_addr, end_addr[
  bool eraseZpp(uint32_t begin_addr, uint32_t end_addr) const final {}

  // Write ZPP
  bool writeZpp(uint32_t addr, std::span<uint8_t const> chunk) final {}

  // Update done
  bool endZpp() final {}

  // Exit (eventually perform CV reset)
  [[noreturn]] void exitZpp(bool reset_cvs) const final {}

  // Timer interrupt calls receive with captured value
  void interrupt() { receive(TIMER_VALUE); }
};

void enter_firmware_update() {}

// Ctor takes ID und function object hooks with void() signature
mdu::rx::entry::Point entry_point{{.decoder_id = ID,
                                   .firmware_entry = enter_firmware_update,
                                   .zpp_entry = nullptr}};

int main() {
  ZppLoad zpp_load{};
  entry_point.verify(0uz, 0u);
}