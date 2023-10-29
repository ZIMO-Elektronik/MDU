#pragma once

#include <filesystem>
#include <source_location>

std::filesystem::path source_location_parent_path(
  std::source_location location = std::source_location::current());
