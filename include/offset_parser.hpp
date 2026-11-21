/*
 * HEXterminator - Terminating your hex editing problems
 * Copyright (C) 2008 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HT_OFFSET_PARSER_HPP
#define HT_OFFSET_PARSER_HPP

#include "byte_offset.hpp"

#include <charconv>
#include <limits>
#include <optional>
#include <string_view>
#include <system_error>

namespace ht::util {

enum class OffsetBase { Decimal = 10, Hexadecimal = 16 };

// Parses decimal or hexadecimal text without surrounding whitespace.
// Returns std::nullopt for invalid syntax and clamps out-of-range values to the nearest ByteOffset limit.
[[nodiscard]] inline std::optional<core::ByteOffset> ParseOffset(std::string_view text, OffsetBase base) {
   const bool negative = !text.empty() && text.front() == '-';

   if (negative)
      text.remove_prefix(1);

   if (base == OffsetBase::Hexadecimal && (text.substr(0, 2) == "0x" || text.substr(0, 2) == "0X"))
      text.remove_prefix(2);

   if (text.empty() || text.front() == '-')
      return std::nullopt;

   core::ByteOffset magnitude = 0;
   const char *end = text.data() + text.size();
   const auto [parsedEnd, error] = std::from_chars(text.data(), end, magnitude, static_cast<int>(base));

   if (parsedEnd != end)
      return std::nullopt;

   if (error == std::errc::result_out_of_range)
      return negative ? std::numeric_limits<core::ByteOffset>::min() : std::numeric_limits<core::ByteOffset>::max();

   return negative ? -magnitude : magnitude;
}

}  // namespace ht::util

#endif  // HT_OFFSET_PARSER_HPP
