/*
 * HEXterminator - Terminating your hex editing problems
 * Copyright (C) 2008 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include "colors.hpp"

#include <wx/settings.h>

namespace {

struct Palette {
   wxColour jumpHexText;
   wxColour jumpDecimalText;
   wxColour rulerBackground;
   wxColour rulerText;
   wxColour highlightBackground;
   wxColour editPosition;
   wxColour separator;
   wxColour editText;
};

const Palette LightPalette {
   wxColour(160, 0, 0),
   wxColour(0, 64, 180),
   wxColour(192, 224, 240),
   wxColour(128, 0, 0),
   wxColour(255, 160, 128),
   wxColour(0, 102, 153),
   wxColour(0, 105, 153),
   wxColour(255, 255, 255),
};

const Palette DarkPalette {
   wxColour(255, 128, 128),
   wxColour(128, 176, 255),
   wxColour(48, 56, 64),
   wxColour(255, 160, 160),
   wxColour(112, 64, 64),
   wxColour(0, 102, 153),
   wxColour(96, 160, 176),
   wxColour(255, 255, 255),
};

std::size_t toIndex(ColorRole role) {
   return static_cast<std::size_t>(role);
}

}  // namespace

const wxColour &Colors::get(ColorRole role) const {
   return m_values[toIndex(role)];
}

void Colors::refreshFromSystemTheme() {
   const Palette &palette = wxSystemSettings::GetAppearance().IsDark() ? DarkPalette : LightPalette;

   m_values[toIndex(ColorRole::JumpHexText)] = palette.jumpHexText;
   m_values[toIndex(ColorRole::JumpDecimalText)] = palette.jumpDecimalText;
   m_values[toIndex(ColorRole::ViewBackground)] = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
   m_values[toIndex(ColorRole::ViewText)] = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
   m_values[toIndex(ColorRole::RulerBackground)] = palette.rulerBackground;
   m_values[toIndex(ColorRole::RulerText)] = palette.rulerText;
   m_values[toIndex(ColorRole::HighlightBackground)] = palette.highlightBackground;
   m_values[toIndex(ColorRole::EditPosition)] = palette.editPosition;
   m_values[toIndex(ColorRole::Separator)] = palette.separator;
   m_values[toIndex(ColorRole::EditText)] = palette.editText;
}
