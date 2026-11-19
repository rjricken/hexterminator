/*
 * HEXterminator - Terminating your hex editing problems
 * Copyright (C) 2008 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef COLORS_HPP
#define COLORS_HPP

#include <array>
#include <cstddef>
#include <wx/colour.h>

enum class ColorRole {
   JumpHexText,
   JumpDecimalText,
   ViewBackground,
   ViewText,
   RulerBackground,
   RulerText,
   HighlightBackground,
   EditPosition,
   Separator,
   EditText,
   Count
};

class Colors {
public:
   const wxColour &get(ColorRole role) const;
   void refreshFromSystemTheme();

private:
   static constexpr std::size_t roleCount = static_cast<std::size_t>(ColorRole::Count);
   std::array<wxColour, roleCount> m_values;
};

#endif  // COLORS_HPP
