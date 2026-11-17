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
};

const Palette LightPalette {
   wxColour(160, 0, 0),
   wxColour(0, 64, 180),
};

const Palette DarkPalette {
   wxColour(255, 128, 128),
   wxColour(128, 176, 255),
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
}
