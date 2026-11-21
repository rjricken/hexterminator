// SPDX-License-Identifier: GPL-2.0-or-later

#include "editor_controller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>

using ht::core::ByteOffset;
using ht::core::EditorController;
using ht::core::EditorState;

TEST_CASE("Jump to offset supports the first byte, last byte, and EOF", "[core][jump]") {
   constexpr ByteOffset length = 32;
   EditorState state;
   EditorController controller(state);

   CHECK(controller.JumpToOffset(length - 1, length) == length - 1);
   CHECK(state.caretOffset == length - 1);

   CHECK(controller.JumpToOffset(length, length) == length);
   CHECK(state.caretOffset == length);

   CHECK(controller.JumpToOffset(0, length) == 0);
   CHECK(state.caretOffset == 0);
}

TEST_CASE("Jump to offset clamps requests outside the document", "[core][jump]") {
   constexpr ByteOffset length = 32;
   EditorState state;
   EditorController controller(state);

   CHECK(controller.JumpToOffset(std::numeric_limits<ByteOffset>::max(), length) == length);
   CHECK(state.caretOffset == length);

   CHECK(controller.JumpToOffset(-1, length) == 0);
   CHECK(state.caretOffset == 0);
}

TEST_CASE("Jump to offset has only one caret position in an empty document", "[core][jump]") {
   EditorState state;
   EditorController controller(state);

   CHECK(controller.JumpToOffset(0, 0) == 0);
   CHECK(controller.JumpToOffset(1, 0) == 0);
   CHECK(controller.JumpToOffset(-1, 0) == 0);
   CHECK(state.caretOffset == 0);
}
