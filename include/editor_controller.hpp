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

struct EditorState {
   ByteOffset caretOffset = 0;
};

class EditorController {
public:
   explicit EditorController(EditorState &state) : m_state(state) {}

   // Requires documentLength >= 0. Clamps requestedOffset to [0, documentLength],
   // then stores and returns the resulting caret offset.
   ByteOffset JumpToOffset(ByteOffset requestedOffset, ByteOffset documentLength) {
      assert(documentLength >= 0);
      m_state.caretOffset = std::clamp(requestedOffset, ByteOffset {0}, documentLength);

      return m_state.caretOffset;
   }

private:
   EditorState &m_state;
};

}  // namespace ht::core

#endif  // HT_EDITOR_CONTROLLER_HPP
