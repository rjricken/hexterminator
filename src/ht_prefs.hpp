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
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HT_PREFS_HPP
#define HT_PREFS_HPP

#include <map>

/**
* Stores the application settings and preferences, allowing it to be
* saved/loaded from an external XML file.
*/
class HtPrefs
{
public:
   HtPrefs ()
   { setDefaultValues(); }

   void setDefaultValues ()
   {
      // preferences
      values[wxT("document/visualization/columns")]         = wxT("16");
      values[wxT("document/visualization/group-by")]        = wxT("4");
      values[wxT("workspace/display/show-ruler")]           = wxT("true");
      values[wxT("workspace/display/show-cursor-position")] = wxT("true");
      values[wxT("workspace/display/show-mouse-position")]  = wxT("true");
      values[wxT("workspace/display/scroll-past-ends")]     = wxT("true");

      // ui-state
      values[wxT("window/maximized")]    = wxT("true");
      values[wxT("window/centered")]     = wxT("true");
      values[wxT("window/position-x")]   = wxT("0");
      values[wxT("window/position-y")]   = wxT("0");
      values[wxT("window/size-x")]       = wxT("450");
      values[wxT("window/size-y")]       = wxT("460");
   }

   /**
   * Gets the value of the specified property.
   * @param id property name
   * @return The property value (as wxString).
   */
   wxString get (const wxString &id) { return values[id]; }

   /**
   * Gets the boolean value of the specified property. Optionally,
   * you can change the default comparison string.
   * @param id property name
   * @param comp (optional) comparison string
   * @return The property value (as a bool).
   */
   bool getBool (const wxString &id, const wxChar *comp = wxT("true"))
   { return values[id] == comp; }

   /**
   * Gets the value of the specified property converted to integer.
   * @param id property name
   * @return The property value (as int).
   */
   int getInt (const wxString &id)
   { return wxAtoi(values[id]); }


private: 
   std::map <wxString, wxString> values; /*< preferences (name/value pair) */
};

#endif //~HT_PREFS_HPP
