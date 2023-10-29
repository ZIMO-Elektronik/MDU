#include "utility.hpp"

std::filesystem::path
source_location_parent_path(std::source_location location) {
  return std::filesystem::path(location.file_name()).parent_path();
}
