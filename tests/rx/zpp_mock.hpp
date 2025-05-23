#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

struct ZppMock : mdu::rx::ZppBase {
  using mdu::rx::ZppBase::ZppBase;
  MOCK_METHOD(void, ackbit, (uint32_t), (const, override));
  MOCK_METHOD(bool, readCv, (uint32_t, uint32_t), (const, override));
  MOCK_METHOD(bool, writeCv, (uint32_t, uint8_t), (override));
  MOCK_METHOD(bool, zppValid, (std::string_view, size_t), (const, override));
  MOCK_METHOD(bool,
              loadCodeValid,
              ((std::span<uint8_t const, 4uz>)),
              (const, override));
  MOCK_METHOD(bool, eraseZpp, (uint32_t, uint32_t), (override));
  MOCK_METHOD(bool, writeZpp, (uint32_t, std::span<uint8_t const>), (override));
  MOCK_METHOD(bool, endZpp, (), (override));
  MOCK_METHOD(void, exitZpp, (bool), (override));
};
