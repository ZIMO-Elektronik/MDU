# Changelog

## 0.17.0
- ZSU update entry by a CV verify sequence with decoder ID and serial number 0

## 0.16.1
- Bugfix `ESP_GOTO_ON_FALSE` condition

## 0.16.0
- Add `MDU_TX_MAX_PREAMBLE_BITS` definition
- Add `MDU_TX_MAX_ACKREQ_BITS` definition

## 0.15.4
- Bugfix remove extra preamble symbol from encoder

## 0.15.3
- Bugfix documentation

## 0.15.2
- Bugfix logo path

## 0.15.1
- Absolute image paths for [ESP registry](https://components.espressif.com/)

## 0.15.0
- Bugfix RMT encoder ([#13032](https://github.com/espressif/esp-idf/issues/13032))

## 0.14.0
- Consistent spelling of 'load code'

## 0.13.0
- Add ESP32 RMT encoder
- Rename Firmware* to Zsu* (classes, commands, ...)

## 0.12.0
- More sophisticated packet

## 0.11.2
- Don't touch nack in reset()

## 0.11.1
- NACK logic changed slightly
  - Previous versions prepared an implicit nack after each reset
  - Now at least 2 preamble bits must be received so that a NACK is prepared

## 0.11.0
- Methods which potentially mutate state are no longer const
  - `rx::Base::writeCv`
  - `rx::FirmwareBase::eraseFirmware`
  - `rx::FirmwareBase::writeFirmware`
  - `rx::FirmwareBase::exitFirmware`
  - `rx::ZppBase::eraseZpp`
  - `rx::ZppBase::exitZpp`

## 0.10.0
- ZPP-Valid-Query command is mandatory

## 0.9.1
- Use CPM.cmake

## 0.9.0
- Add ZPP-Valid-Query command

## 0.8.0
- CV read/write uses address instead of number

## 0.7.3
- Update to GSL 4.0.0
- Update to ZTL 0.16.0

## 0.7.2
- Update to ZTL 0.15.0

## 0.7.1
- Update to ZTL 0.14.0
- Virtual dtors

## 0.7.0
- [Semantic versioning](https://semver.org)
- Renamed namespace receive to rx
- Renamed namespace transmit to tx

## 0.6
- Update to ZTL 0.13

## 0.5
- Removed ConfigCutout command
- Update to ZTL 0.12

## 0.4
- Inline FirmwareMixin to allow more aggressive optimizations

## 0.3
- Removed CMake exports
- Update to ZTL 0.11

## 0.2
- Update to ZTL 0.9

## 0.1
- First release with classes for entry, firmware- and ZPP update