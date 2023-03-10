#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

struct FirmwareMock : mdu::rx::FirmwareBase {
  using mdu::rx::FirmwareBase::FirmwareBase;
  MOCK_METHOD(void, ackbit, (uint32_t), (const));
  MOCK_METHOD(bool, readCv, (uint32_t, uint32_t), (const));
  MOCK_METHOD(bool, writeCv, (uint32_t, uint8_t));
  MOCK_METHOD(bool, eraseFirmware, (uint32_t, uint32_t));
  MOCK_METHOD(bool,
              writeFirmware,
              (uint32_t, (std::span<uint8_t const, 64uz>)),
              ());
  MOCK_METHOD(void, exitFirmware, ());
};