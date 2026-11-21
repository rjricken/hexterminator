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

#ifndef HT_EDITOR_CONTROLLER_HPP
#define HT_EDITOR_CONTROLLER_HPP

#include "byte_offset.hpp"

#include <algorithm>
#include <cassert>

namespace ht::core {

enum class EditorPane { Hex, Character };

enum class CaretMove { Left, Right, Up, Down, DocumentStart, DocumentEnd };

struct EditorState {
   ByteOffset caretOffset = 0;
   EditorPane activePane = EditorPane::Hex;
};

class EditorController {
public:
   const EditorState &GetState() const { return m_state; }

   void SwitchPane() {
      m_state.activePane = m_state.activePane == EditorPane::Hex ? EditorPane::Character : EditorPane::Hex;
   }

   void PlaceCaret(ByteOffset requestedOffset, EditorPane pane, ByteOffset documentLength) {
      JumpToOffset(requestedOffset, documentLength);
      m_state.activePane = pane;
   }

   ByteOffset MoveCaret(CaretMove move, ByteOffset documentLength, int bytesPerRow) {
      assert(documentLength >= 0 && bytesPerRow > 0);
      ByteOffset offset = std::min(m_state.caretOffset, documentLength);

      switch (move) {
         case CaretMove::Left:
            if (offset > 0)
               --offset;
            break;
         case CaretMove::Right:
            if (offset < documentLength)
               ++offset;
            break;
         case CaretMove::Up:
            if (offset >= bytesPerRow)
               offset -= bytesPerRow;
            break;
         case CaretMove::Down:
            offset += std::min<ByteOffset>(bytesPerRow, documentLength - offset);
            break;
         case CaretMove::DocumentStart:
            offset = 0;
            break;
         case CaretMove::DocumentEnd:
            offset = documentLength;
            break;
      }

      m_state.caretOffset = offset;
      return offset;
   }

   // Requires documentLength >= 0. Clamps requestedOffset to [0, documentLength],
   // then stores and returns the resulting caret offset.
   ByteOffset JumpToOffset(ByteOffset requestedOffset, ByteOffset documentLength) {
      assert(documentLength >= 0);
      m_state.caretOffset = std::clamp(requestedOffset, ByteOffset {0}, documentLength);

      return m_state.caretOffset;
   }

private:
   EditorState m_state;
};

}  // namespace ht::core

#endif  // HT_EDITOR_CONTROLLER_HPP
