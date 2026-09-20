#pragma once
#include <cstddef>
#include <span>
#include <vector>

namespace matf::verification::metamorphic_testing::pdf {


std::vector<std::vector<std::byte>> split_pages(std::span<const std::byte> pdf);


} // namespace matf::verification::metamorphic_testing::pdf
