// SPDX-License-Identifier: GPL-2.0-or-later

#include "editor_controller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>

using ht::core::ByteOffset;
using ht::core::CaretMove;
using ht::core::EditorController;
using ht::core::EditorPane;

TEST_CASE("Jump to offset supports the first byte, last byte, and EOF", "[core][jump]") {
   constexpr ByteOffset length = 32;
   EditorController controller;
   const auto &state = controller.GetState();

   CHECK(controller.JumpToOffset(length - 1, length) == length - 1);
   CHECK(state.caretOffset == length - 1);

   CHECK(controller.JumpToOffset(length, length) == length);
   CHECK(state.caretOffset == length);

   CHECK(controller.JumpToOffset(0, length) == 0);
   CHECK(state.caretOffset == 0);
}

TEST_CASE("Jump to offset clamps requests outside the document", "[core][jump]") {
   constexpr ByteOffset length = 32;
   EditorController controller;
   const auto &state = controller.GetState();

   CHECK(controller.JumpToOffset(std::numeric_limits<ByteOffset>::max(), length) == length);
   CHECK(state.caretOffset == length);

   CHECK(controller.JumpToOffset(-1, length) == 0);
   CHECK(state.caretOffset == 0);
}

TEST_CASE("Jump to offset has only one caret position in an empty document", "[core][jump]") {
   EditorController controller;
   const auto &state = controller.GetState();

   CHECK(controller.JumpToOffset(0, 0) == 0);
   CHECK(controller.JumpToOffset(1, 0) == 0);
   CHECK(controller.JumpToOffset(-1, 0) == 0);
   CHECK(state.caretOffset == 0);
}

TEST_CASE("Placing the caret selects the requested pane even when clamped to EOF", "[core][navigation]") {
   EditorController controller;
   controller.PlaceCaret(12, EditorPane::Character, 10);
   CHECK(controller.GetState().caretOffset == 10);
   CHECK(controller.GetState().activePane == EditorPane::Character);

   controller.JumpToOffset(4, 10);
   CHECK(controller.GetState().activePane == EditorPane::Character);
}

TEST_CASE("Switching panes preserves the caret offset in both directions", "[core][navigation]") {
   EditorController controller;
   controller.JumpToOffset(4, 10);

   controller.SwitchPane();
   CHECK(controller.GetState().activePane == EditorPane::Character);
   CHECK(controller.GetState().caretOffset == 4);

   controller.SwitchPane();
   CHECK(controller.GetState().activePane == EditorPane::Hex);
   CHECK(controller.GetState().caretOffset == 4);
}

TEST_CASE("Keyboard moves cross rows and stop at document boundaries", "[core][navigation]") {
   EditorController controller;
   constexpr int width = 4;
   constexpr ByteOffset length = 10;
   auto move = [&](CaretMove direction) { return controller.MoveCaret(direction, length, width); };

   controller.PlaceCaret(3, EditorPane::Character, length);
   CHECK(move(CaretMove::Up) == 3);
   CHECK(move(CaretMove::Right) == 4);
   CHECK(move(CaretMove::Left) == 3);
   CHECK(move(CaretMove::Down) == 7);
   CHECK(move(CaretMove::Down) == length);
   CHECK(move(CaretMove::Right) == length);
   CHECK(move(CaretMove::Up) == 6);
   CHECK(move(CaretMove::DocumentStart) == 0);
   CHECK(move(CaretMove::Left) == 0);
   CHECK(move(CaretMove::DocumentEnd) == length);
   CHECK(controller.GetState().activePane == EditorPane::Character);
   CHECK(controller.GetState().caretOffset == length);
}

TEST_CASE("Keyboard moves handle empty documents, aligned EOF and large offsets", "[core][navigation]") {
   EditorController controller;
   for (auto direction : {CaretMove::Left,
           CaretMove::Right,
           CaretMove::Up,
           CaretMove::Down,
           CaretMove::DocumentStart,
           CaretMove::DocumentEnd})
      CHECK(controller.MoveCaret(direction, 0, 4) == 0);

   CHECK(controller.MoveCaret(CaretMove::DocumentEnd, 8, 4) == 8);
   CHECK(controller.MoveCaret(CaretMove::Up, 8, 4) == 4);
   CHECK(controller.MoveCaret(CaretMove::Down, 8, 4) == 8);

   constexpr auto limit = std::numeric_limits<ByteOffset>::max();
   controller.JumpToOffset(limit - 1, limit);
   CHECK(controller.MoveCaret(CaretMove::Down, limit, 16) == limit);
   CHECK(controller.MoveCaret(CaretMove::Right, limit, 16) == limit);
}
