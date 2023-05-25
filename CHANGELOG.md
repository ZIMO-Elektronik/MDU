# Changelog

# 0.11.1
- NACK logic changed slightly
  - Previous versions prepared an implicit nack after each reset
  - Now at least 2 preamble bits must be received so that a NACK is prepared

# 0.11.0
- Methods which potentially mutate state are no longer const
  - rx::Base::writeCv
  - rx::FirmwareBase::eraseFirmware
  - rx::FirmwareBase::writeFirmware
  - rx::FirmwareBase::exitFirmware
  - rx::ZppBase::eraseZpp
  - rx::ZppBase::exitZpp

# 0.10.0
- ZPP-Valid-Query command is mandatory

# 0.9.1
- Use CPM.cmake

# 0.9.0
- Add ZPP-Valid-Query command

# 0.8.0
- CV read/write uses address instead of number

# 0.7.3
- Update to GSL 4.0.0
- Update to ZTL 0.16.0

# 0.7.2
- Update to ZTL 0.15.0

# 0.7.1
- Update to ZTL 0.14.0
- Virtual dtors

# 0.7.0
- [Semantic versioning](https://semver.org)
- Renamed namespace receive to rx
- Renamed namespace transmit to tx

# 0.6
- Update to ZTL 0.13

# 0.5
- Removed ConfigCutout command
- Update to ZTL 0.12

# 0.4
- Inline FirmwareMixin to allow more aggressiv optimizations

# 0.3
- Removed CMake exports
- Update to ZTL 0.11

# 0.2
- Update to ZTL 0.9

# 0.1
- First release with classes for entry, firmware- and ZPP update