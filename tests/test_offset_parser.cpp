// SPDX-License-Identifier: GPL-2.0-or-later

#include "offset_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>

using ht::core::ByteOffset;
using ht::util::OffsetBase;
using ht::util::ParseOffset;

TEST_CASE("ParseOffset reads signed decimal and hexadecimal offsets", "[util][offset]") {
   SECTION("decimal") {
      CHECK(ParseOffset("0", OffsetBase::Decimal) == 0);
      CHECK(ParseOffset("42", OffsetBase::Decimal) == 42);
      CHECK(ParseOffset("-42", OffsetBase::Decimal) == -42);
   }

   SECTION("hexadecimal") {
      CHECK(ParseOffset("2A", OffsetBase::Hexadecimal) == 42);
      CHECK(ParseOffset("0x2a", OffsetBase::Hexadecimal) == 42);
      CHECK(ParseOffset("-0X2A", OffsetBase::Hexadecimal) == -42);
   }
}

TEST_CASE("ParseOffset clamps valid numbers to the ByteOffset limits", "[util][offset]") {
   constexpr auto minOffset = std::numeric_limits<ByteOffset>::min();
   constexpr auto maxOffset = std::numeric_limits<ByteOffset>::max();

   CHECK(ParseOffset("9223372036854775807", OffsetBase::Decimal) == maxOffset);
   CHECK(ParseOffset("9223372036854775808", OffsetBase::Decimal) == maxOffset);
   CHECK(ParseOffset("-9223372036854775808", OffsetBase::Decimal) == minOffset);
   CHECK(ParseOffset("-9223372036854775809", OffsetBase::Decimal) == minOffset);
   CHECK(ParseOffset("0x8000000000000000", OffsetBase::Hexadecimal) == maxOffset);
   CHECK(ParseOffset("-0x8000000000000001", OffsetBase::Hexadecimal) == minOffset);
}

TEST_CASE("ParseOffset rejects malformed input", "[util][offset]") {
   SECTION("decimal") {
      constexpr const char *invalidInputs[] = {"", "-", "--0", "+1", " 42", "42 ", "0x2A", "9223372036854775808x"};

      for (const char *text : invalidInputs) {
         CAPTURE(text);
         CHECK_FALSE(ParseOffset(text, OffsetBase::Decimal).has_value());
      }
   }

   SECTION("hexadecimal") {
      constexpr const char *invalidInputs[] = {"0x", "-0x", "0x-0", "-0x-0", "0x+1", "0x8000000000000000z"};

      for (const char *text : invalidInputs) {
         CAPTURE(text);
         CHECK_FALSE(ParseOffset(text, OffsetBase::Hexadecimal).has_value());
      }
   }
}
